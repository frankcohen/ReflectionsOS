/*
  Reflections is a hardware and software platform for building entertaining mobile experiences.

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com

  This module performs real-time detection of single and double tap gestures
  using the LIS3DH accelerometer.

  (Header omitted for brevity - unchanged from your original.)
*/

#include "AccelSensor.h"

#include "driver/rtc_io.h"

// -------------------------------
// Tap-to-abort tuning (runtime)
// -------------------------------
// Runtime click engine tuning (awake mode)
static const uint8_t RUNTIME_CLICK_THRESHOLD = 25; // raise vs 20 to reduce sensitivity

// At 400 Hz: 1 count = 2.5 ms
static const uint8_t RUNTIME_TIME_LIMIT   = 10; // 25 ms
static const uint8_t RUNTIME_TIME_LATENCY = 10; // 90 ms dead-time (ringing suppression)
static const uint8_t RUNTIME_TIME_WINDOW  = 200; // 200 ms (keep tight)

// Stillness gate (m/s^2). Lower = stricter stillness requirement.
static const float STILLNESS_DEV_MAX = 0.75f;

// Magnitude EMA smoothing
static const float MAG_EMA_ALPHA     = 0.08f;
static const float MAG_DEV_EMA_ALPHA = 0.12f;

// Shortest signed angular delta between a and b (degrees), result in [-180, +180]
static inline float shortestAngleDelta(float a_deg, float b_deg) {
  float d = a_deg - b_deg;
  while (d > 180.0f) d -= 360.0f;
  while (d < -180.0f) d += 360.0f;
  return d;
}

// Compute the "twist angle" in degrees based on selected axis mode.
// Mode 0: roll-like angle = atan2(Y, Z)
// Mode 1: alternative roll = atan2(X, Z) (often better if the board is rotated)
static inline float computeTwistAngleDeg(const sensors_event_t& e) {
#if TWIST_AXIS_MODE == 1
  return (180.0f / PI) * atan2f(e.acceleration.x, e.acceleration.z);
#else
  return (180.0f / PI) * atan2f(e.acceleration.y, e.acceleration.z);
#endif
}

// ——— register names from Adafruit_LIS3DH.h ———
//  INT1CFG: enable X/Y/Z high-event interrupts on INT1
//  INT1THS: threshold
//  INT1DUR: duration
//  CTRL_REG5: interrupt latch enable
// ————————————————————————————————

// raw write to any LIS3DH register over I²C
static void write8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ACCEL_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

AccelSensor::AccelSensor() {}

void AccelSensor::configureRuntimeTapProfile()
{
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);
  lis.setRange(LIS3DH_RANGE_8_G);

  // Enable DOUBLE-click detection at runtime (not single)
  lis.setClick(2, RUNTIME_CLICK_THRESHOLD, RUNTIME_TIME_LIMIT, RUNTIME_TIME_LATENCY, RUNTIME_TIME_WINDOW);
  write8(LIS3DH_REG_CLICKCFG, 0x3F);   // XYZ single+double (wide-open)

  // Route click interrupt to INT1
  write8(LIS3DH_REG_CTRL3, 0x80);

  lis.getClick(); // clear pending
}

void AccelSensor::configureWakeTapProfile()
{
  configureSensorWakeOnMotion();
}

void AccelSensor::updateStillnessEstimate(float mag)
{
  if (_magEma == 0.0f) _magEma = mag;
  _magEma += MAG_EMA_ALPHA * (mag - _magEma);

  float dev = fabsf(mag - _magEma);
  _magDevEma += MAG_DEV_EMA_ALPHA * (dev - _magDevEma);
}

bool AccelSensor::getSingleTapAbortCandidateNoClear()
{
  if (!_pendingSingle) return false;
  if (!_lastClickHadZ) return false;
  if (_magDevEma > STILLNESS_DEV_MAX) return false;
  return true;
}

bool AccelSensor::getSingleTapAbortCandidate()
{
  bool ok = getSingleTapAbortCandidateNoClear();
  if (ok) _pendingSingle = false; // consume
  return ok;
}

void AccelSensor::begin() {
  started = false;
  runflag = false;

  lis = Adafruit_LIS3DH();

  if (!lis.begin(accelAddress)) {
    Serial.println(F("Accelerometer did not start, stopping"));
    video.stopOnError("Accel", "did not", "start", " ", " ");
  }

  magtime = millis();
  last = millis();

  started = false;
  runflag = false;

  firstpart = false;
  firstClickTime = millis();
  minClickTime = millis();
  waittime = millis();
  _pendingSingle = false;
  _pendingDouble = false;
  _twistIgnoreUntilMs = 0;

  // New gating state
  _lastClickHadZ = false;
  _lastClickMs = 0;
  _magEma = 0.0f;
  _magDevEma = 0.0f;

  shakentime = millis();
  shakentime2 = millis();
  shakencount = 0;

  // IMPORTANT: while awake, use runtime profile.
  // Switch to wake profile ONLY right before deep sleep.
  configureRuntimeTapProfile();

  started = true;

  // ---- Initialize wrist twist state ----
  _rollEma = 0.0f;
  _lastRoll = 0.0f;
  _twistAccumDeg = 0.0f;
  _twistStartMs = millis();
  _twistLastMs = millis();
  _twistCooldownUntil = 0;
  _twistArmed = false;
  _twistPending = false;
  _twistDbgLastMs = 0;

  _ax = _ay = _az = 0.0f;
  _twistDirHits = 0;
  _twistDirLast = 0;

  // Seed initial roll from a single read (helps EMA converge quickly)
  {
    sensors_event_t seedEvt;
    lis.getEvent(&seedEvt);
    float rollNow = computeTwistAngleDeg(seedEvt);
    _rollEma = rollNow;
    _lastRoll = rollNow;
  }
}

bool AccelSensor::getWristTwist() {
  bool hit = _twistPending;
  _twistPending = false;
  return hit;
}

int AccelSensor::getWristTwistDir()
{
  // Don’t report twist direction near taps
  if (millis() < _twistIgnoreUntilMs)
  {
    static float lastX = 0;
    static float lastY = 0;
    lastX = _ax;
    lastY = _ay;
    _twistDirHits = 0;
    _twistDirLast = 0;
    return 0;
  }

  static float lastX = 0;
  static float lastY = 0;

  float x = getXreading();
  float y = getYreading();

  float dx = x - lastX;
  float dy = y - lastY;

  lastX = x;
  lastY = y;

  float twist = (dx - dy);

  const float DEADZONE = 0.35f;

  int dir = 0;
  if (twist > DEADZONE) dir = +1;
  else if (twist < -DEADZONE) dir = -1;

  if (dir == 0) { _twistDirHits = 0; _twistDirLast = 0; return 0; }

  if (dir == _twistDirLast) _twistDirHits++;
  else { _twistDirLast = dir; _twistDirHits = 1; }

  if (_twistDirHits >= 2) { _twistDirHits = 0; _twistDirLast = 0; return dir; }

  return 0;
}

bool AccelSensor::isShaken()
{
  if (shakencount > SHAKEN_COUNT)
  {
    shakencount = 0;
    return true;
  }

  return false;
}

float AccelSensor::getXreading() { return started ? _ax : 0; }
float AccelSensor::getYreading() { return started ? _ay : 0; }
float AccelSensor::getZreading() { return started ? _az : 0; }

void AccelSensor::configureSensorWakeOnMotion()
{
  // Keep a known ODR so timing math is stable
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);

  // Click timing registers are in samples (LSB = 1/ODR).
  // At 400 Hz: 1 LSB = 2.5 ms
  const float msPerCount = 1000.0f / 400.0f;

  uint8_t timeLimit   = (uint8_t)constrain((int)round(WAKE_MIN_MOTION_MS / msPerCount), 1, 255);
  uint8_t timeLatency = (uint8_t)constrain((int)round(WAKE_LATENCY_MS    / msPerCount), 0, 255);
  uint8_t timeWindow  = (uint8_t)constrain((int)round(WAKE_MAX_WINDOW_MS / msPerCount), 1, 255);

  lis.setRange(LIS3DH_RANGE_8_G);

  // Wake uses DOUBLE-click (your existing design)
  lis.setClick(2, CLICKTHRESHHOLD, timeLimit, timeLatency, timeWindow);

  // Enable double-click on X, Y, Z (per your prior config)
  write8(LIS3DH_REG_CLICKCFG, 0x2A);

  // Route click interrupt to INT1 pin (I1_CLICK)
  write8(LIS3DH_REG_CTRL3, 0x80);

  // Clear any pending click (read click-source register).
  // Do it twice with a tiny gap to avoid going to sleep with INT already asserted.
  (void)lis.getClick();
  delay(5);
  (void)lis.getClick();

  // ---- IMPORTANT: define the INT pin level during deep sleep ----
  // Your boards have no external pull resistors on INT1/INT2, so the line can float.
  // EXT1 wake uses the RTC domain; use RTC pulls (more reliable than pinMode pulls).
  rtc_gpio_pullup_dis((gpio_num_t)ACCEL_INT1_PIN);
  rtc_gpio_pulldown_en((gpio_num_t)ACCEL_INT1_PIN);

  // Arm the ESP32-S3 EXT1 wake on a HIGH at ACCEL_INT1_PIN only
  const uint64_t wakeMask = (1ULL << ACCEL_INT1_PIN);
  esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);
}

void AccelSensor::resetTaps() {
  _pendingSingle = false;
  _pendingDouble = false;
}

bool AccelSensor::getSingleTapNoClear() {
  return _pendingSingle;
}

bool AccelSensor::getDoubleTapNoClear() {
  return _pendingDouble;
}

bool AccelSensor::getSingleTap() {
  bool ts = _pendingSingle;
  _pendingSingle = false;
  return ts;
}

bool AccelSensor::getDoubleTap() {
  bool ts = _pendingDouble;
  _pendingDouble = false;
  return ts;
}

String AccelSensor::getRecentMessage() {
  String myMefa = myMef;
  myMef = "";
  return myMefa;
}

String AccelSensor::getRecentMessage2() {
  String myMefa = myMef2;
  myMef2 = "";
  return myMefa;
}

void AccelSensor::setStatus(bool run) {
  runflag = run;
}

bool AccelSensor::getStatus() {
  return runflag;
}

// Detect single and double clicks
void AccelSensor::handleClicks()
{
  unsigned long now = millis();

  // If we saw a single-click candidate, confirm it once the double-click window expires
  const unsigned long CONFIRM_MS = 450;

  if (firstpart && (now - firstClickTime > CONFIRM_MS))
  {
    firstpart = false;
    _pendingSingle = true;
    waittime = now;
    _twistIgnoreUntilMs = now + 1200;
    return;
  }

  // Always read click source so we can suppress twist on tap impulses
  uint8_t click = lis.getClick();
  if (click & 0x30) {
    // Record whether Z contributed (bit 2 per your header comment)
    _lastClickHadZ = (click & 0x04) != 0;
    _lastClickMs = now;

    _twistIgnoreUntilMs = now + 1200;
  }

  // Apply your global quiet time for generating tap events
  if (now - waittime < WAIT_TIME) return;

  if (click == 0) return;
  if (!(click & 0x30)) return;

  if (now - minClickTime < 120) return;
  minClickTime = now;

  bool isDouble = (click & 0x20) != 0;
  bool isSingle = (click & 0x10) != 0;

  if (isDouble)
  {
    _pendingDouble = true;
    _pendingSingle = false;
    firstpart = false;
    waittime = now;
    _twistIgnoreUntilMs = now + 1200;
    return;
  }

  if (isSingle)
  {
    firstClickTime = now;
    firstpart = true;
    _twistIgnoreUntilMs = now + 1200;
    return;
  }
}

void AccelSensor::loop()
{
  if (!runflag) return;

  handleClicks();     // Detect single and double clicks

  if (millis() - last < SAMPLE_RATE) return;
  last = millis();

  sensors_event_t event;
  lis.getEvent(&event);

  _ax = event.acceleration.x;
  _ay = event.acceleration.y;
  _az = event.acceleration.z;

  magnow = sqrt(
    (event.acceleration.x * event.acceleration.x) +
    (event.acceleration.y * event.acceleration.y) +
    (event.acceleration.z * event.acceleration.z)
  );

  // Tap-to-abort stillness estimator
  updateStillnessEstimate(magnow);

  // ---- Wrist Twist Detector (with debug) ----
  {
    unsigned long now = millis();

    if (now < _twistIgnoreUntilMs) {
      _twistArmed = false;
      _twistAccumDeg = 0.0f;
      _twistPending = false;
    }
    else
    {
      float rollDeg = computeTwistAngleDeg(event);
      _rollEma += ROLL_EMA_ALPHA * (rollDeg - _rollEma);

#if TWIST_BYPASS_GRAVITY
      bool nearG = true;
#else
      bool nearG = (magnow >= TWIST_G_MIN) && (magnow <= TWIST_G_MAX);
#endif

      if (now >= _twistCooldownUntil)
      {
        if (!_twistArmed)
        {
          if (nearG)
          {
            _twistArmed = true;
            _twistStartMs = now;
            _twistAccumDeg = 0.0f;
            _lastRoll = _rollEma;
          }
        }

        if (_twistArmed)
        {
          float d = shortestAngleDelta(_rollEma, _lastRoll);

          if (fabsf(d) < 1.5f) d = 0.0f;

          _twistAccumDeg += d;
          _lastRoll = _rollEma;

          unsigned long age = now - _twistStartMs;
          if (age <= TWIST_MAX_WINDOW_MS) {
            if (fabsf(_twistAccumDeg) >= TWIST_MIN_ANGLE_DEG)
            {
              _twistPending = true;
              _twistArmed = false;
              _twistCooldownUntil = now + TWIST_COOLDOWN_MS;
            }
          }
          else
          {
            _twistArmed = false;
            _twistAccumDeg = 0.0f;
          }
        }
      }
      else
      {
        _twistArmed = false;
      }

#if TWIST_DEBUG
      if (now - _twistDbgLastMs >= TWIST_DEBUG_PERIOD_MS)
      {
        _twistDbgLastMs = now;
        long coolLeft = (now >= _twistCooldownUntil) ? 0 : (long)(_twistCooldownUntil - now);
        unsigned long age = now - _twistStartMs;

        Serial.printf(
          "TWIST dbg | t=%lu ms | axis=%s | nearG=%d | armed=%d | pend=%d | cool=%ld "
          "| roll=%.1f ema=%.1f d=%.1f accum=%.1f | mag=%.2f | age=%lu\n",
          now,
#if TWIST_AXIS_MODE == 1
          "XZ",
#else
          "YZ",
#endif
          (int)nearG,
          (int)_twistArmed,
          (int)_twistPending,
          coolLeft,
          rollDeg, _rollEma,
          shortestAngleDelta(_rollEma, _lastRoll),
          _twistAccumDeg,
          magnow,
          age
        );
      }
#endif
    }
  }

  // With enough movement, run the shaken experience
  if (millis() - shakentime > 5000)
  {
    shakentime = millis();
    shakencount = 0;
  }

  if ((magnow > 15) && (millis() - shakentime2 > 200))
  {
    shakentime2 = millis();
    shakencount++;
  }
}