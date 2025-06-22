/*
  TOF Sensor Gesture Detection

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library

  Reflections board uses a Time Of Flight (TOF) VL53L5CX sensor to
  identify user gestures with their fingers and hand. Gestures control
  operating the Experiences.

  Datasheet comes with this source code, see: vl53l5cx-2886943_.pdf
*/

#include "TOF.h"

// Initialize static sleepStart
unsigned long TOF::sleepStart = 0;

TOF::TOF() {}

void TOF::begin() 
{
  if (!tof.begin()) {
    Serial.println("VL53L5CX init failed");
    while (1);
  }

  Serial.println("VL53L5CX started");

  // Configure sensor
  tof.setRangingFrequency(FRAME_RATE);
  tof.setResolution(64);

  if (!tof.startRanging()) 
  {
    Serial.println("TOF failed to start ranging. Stopping");
    while (1) delay(10);
  }

  // Initialize timing
  lastRead    = 0;
  captTime    = millis();
  timeLimit   = millis();
  sleepStart  = millis();

  pendingDirection = false;
  prevValid        = false;
  direction        = GESTURE_NONE;
  tipPos           = 0;
  isRunning        = false;
  direction        = GESTURE_NONE;
  directionWay     = " ";

  // Zero‐initialize frame buffers
  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      frame1[y][x] = 0.0f;
      frame2[y][x] = 0.0f;
    }
  }
}

void TOF::resetGesture() {
  prevValid        = false;
  pendingDirection = false;
  directionWay     = " ";
  direction        = GESTURE_NONE;
}

// Motion estimation is a Lucas–Kanade optical‐flow solver 

void TOF::computeOpticalFlow(const float frameA[HEIGHT][WIDTH],
                             const float frameB[HEIGHT][WIDTH],
                             float &u, float &v)
{
  double sumIx2  = 0.0;
  double sumIy2  = 0.0;
  double sumIxIy = 0.0;
  double sumIxIt = 0.0;
  double sumIyIt = 0.0;

  // Iterate interior pixels (1..6 in both x and y)
  for (int y = 1; y < HEIGHT - 1; ++y) {
    for (int x = 1; x < WIDTH - 1; ++x) {
      float Ix = (
          (frameA[y][x + 1] - frameA[y][x - 1]) +
          (frameB[y][x + 1] - frameB[y][x - 1])
        ) * 0.25f;

      float Iy = (
          (frameA[y + 1][x] - frameA[y - 1][x]) +
          (frameB[y + 1][x] - frameB[y - 1][x])
        ) * 0.25f;

      float It = frameB[y][x] - frameA[y][x];

      sumIx2  += Ix * Ix;
      sumIy2  += Iy * Iy;
      sumIxIy += Ix * Iy;
      sumIxIt += Ix * It;
      sumIyIt += Iy * It;
    }
  }

  double A00 = sumIx2;
  double A01 = sumIxIy;
  double A10 = sumIxIy;
  double A11 = sumIy2;
  double b0  = -sumIxIt;
  double b1  = -sumIyIt;

  double det = A00 * A11 - A01 * A10;
  if (fabs(det) < 1e-6) {
    u = 0.0f;
    v = 0.0f;
    return;
  }

  double invA00 =  A11 / det;
  double invA01 = -A01 / det;
  double invA10 = -A10 / det;
  double invA11 =  A00 / det;

  double u_d = invA00 * b0 + invA01 * b1;
  double v_d = invA10 * b0 + invA11 * b1;

  u = static_cast<float>(u_d);
  v = static_cast<float>(v_d);
}

int TOF::getGesture() 
{
  int prevd = direction;
  direction = 0;
  return prevd;
}

void TOF::setStatus(bool running) {
  isRunning = running;
  if ( isRunning )
  {
    tof.startRanging();
  }
  else
  {
    tof.stopRanging();
  }
}

bool TOF::getStatus() {
  return isRunning;
}

int TOF::getFingerPos() {
  return tipPos;
}

float TOF::getFingerDist(){
  return tipDist;
}

String TOF::getRecentMessage()
{
  String myMefa = mymessage;
  mymessage = "";
  return myMefa;
}

String TOF::getRecentMessage2()
{
  String myMefa = mymessage2;
  mymessage2 = "";
  return myMefa;
}

void TOF::loop() {
  if (!isRunning) {
    return;
  }

  unsigned long now = millis();

  // Enforce 2 s lockout after a gesture is reported
  if (now - captTime < 2000) {
    return;
  }

  // Wait until sensor has new data
  if (!tof.isDataReady()) {
    return;
  }

  // Throttle read rate to FRAME_INTERVAL_MS
  if (now - lastRead < FRAME_INTERVAL_MS) {
    return;
  }
  lastRead = now;

  // If no direction is pending and 1 s has passed since timeLimit, reset state
  if ((!pendingDirection) && (now - timeLimit > 1000)) {
    timeLimit = now;
    resetGesture();
  }

  // Fetch new 8×8 frame
  VL53L5CX_ResultsData rd;
  if ( !tof.getRangingData(&rd) ) return;

  // Build raw[64] and count how many pixels lie in [MIN_DISTANCE, MAX_DISTANCE]
  int16_t raw[64];
  int validCnt = 0;
  for (int i = 0; i < 64; ++i) {
    raw[i] = rd.distance_mm[i];
    if (raw[i] >= MIN_DISTANCE && raw[i] <= MAX_DISTANCE) {
      validCnt++;
    }
  }

  // ——— Preprocess: Rotate 90° CW then vertical flip ———
  int16_t rotated[64];
  // Step 1: Transpose (r,c) → (c,r)
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      rotated[c * 8 + r] = raw[r * 8 + c];
    }
  }
  // Step 2: Vertical flip
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 8; ++c) {
      int topIdx    = r * 8 + c;
      int bottomIdx = (7 - r) * 8 + c;
      int16_t tmp   = rotated[topIdx];
      rotated[topIdx]    = rotated[bottomIdx];
      rotated[bottomIdx] = tmp;
    }
  }

  int16_t minDist = TIP_MAX_DISTANCE + 1;
  tipPos = 0;

  // Fingertip detection: Phase 1 (center of valid mass) or Phase 2 (minimum value)
  bool leftInvalid = false;
  bool rightInvalid = false;

  // Check leftmost and rightmost valid columns (columns 0 and 5 only)
  for (int row = 0; row < 8; row++) {
    int16_t leftVal  = rotated[row * 8 + 0];
    int16_t rightVal = rotated[row * 8 + 5];
    if (leftVal < TIP_MIN_DISTANCE || leftVal > TIP_MAX_DISTANCE) leftInvalid = true;
    if (rightVal < TIP_MIN_DISTANCE || rightVal > TIP_MAX_DISTANCE) rightInvalid = true;
  }

  const uint8_t row = 3; // Central row
  tipPos = -1;           // -1 means not detected

  // Phase 1: Estimate center of valid cluster
  if (leftInvalid || rightInvalid) {
    int validSum = 0;
    int weightedSum = 0;
    for (uint8_t c = 0; c < 6; c++) {
      int16_t d = rotated[row * 8 + c];
      if (d >= TIP_MIN_DISTANCE && d <= TIP_MAX_DISTANCE) {
        validSum++;
        weightedSum += c;
      }
    }
    if (validSum > 0)
    {
      tipPos = weightedSum / validSum;
      tipDist = rotated[row * 8 + tipPos];
    }

  } else {
    // Phase 2: All center columns valid, find the lowest distance value
    for (uint8_t c = 0; c < 6; c++) {
      int16_t d = rotated[row * 8 + c];
      if (d >= TIP_MIN_DISTANCE && d <= TIP_MAX_DISTANCE && d < minDist) {
        minDist = d;
        tipPos = c;
        tipDist = d;
      }
    }
  }

  /*
  mymessage = "tipPos: ";
  mymessage += tipPos;
  mymessage2 = "tipDist: ";
  mymessage2 += tipDist;
  */

  // ——— Sleep detection ———
  if ( now - sleepStart > SLEEP_HOLD_MS )
  {
    sleepStart = now;

    int sleepCnt = 0;
    for (uint8_t r = 0; r < 8; ++r) {
      for (uint8_t c = 0; c < 8; ++c) {
        uint16_t d = rd.distance_mm[ ( r * 8 ) + c];
        if (d >= SLEEP_MIN_DISTANCE && d <= SLEEP_MAX_DISTANCE) {
          sleepCnt++;
        }
      }
    }
    if (sleepCnt >= SLEEP_COVERAGE_COUNT) 
    {
      resetGesture();
      direction = GESTURE_SLEEP;
      directionWay = "→ Sleep";
      mymessage = directionWay;
      mymessage2 = sleepCnt;
      captTime = now;
      return;
    }
  }

  // ——— Pending direction / circular detection ———
  if (pendingDirection) {
    // If still “valid” frames after CIRCULAR_TIME, declare circular
    if ((validCnt > VALID_PIXEL_COUNT) && (now - pendingTimer > CIRCULAR_TIME)) 
    {
      resetGesture();
      direction = GESTURE_CIRCULAR;
      directionWay = "→ Circular";
      mymessage = directionWay;
      captTime = now;
      return;
    }
    // If pixels drop below threshold after CIRCULAR_TIME, finalize original swipe
    if ((validCnt < VALID_PIXEL_COUNT) && (now - pendingTimer > CIRCULAR_TIME)) 
    {
      resetGesture();
      mymessage = directionWay;
      captTime = now;
      return;
    }
    return;
  }

  // If too few valid pixels, do nothing
  if (validCnt < VALID_PIXEL_COUNT) {
    return;
  }

  // ——— Build optical‐flow frames ———
  if (!prevValid) {
    // Store as “previous” frame
    prevValid = true;
    for (int i = 0; i < 64; ++i) {
      int row = i / WIDTH;
      int col = i % WIDTH;
      frame1[row][col] = static_cast<float>(rotated[i]);
    }
    return;
  }

  // Store as “current” frame
  for (int i = 0; i < 64; ++i) {
    int row = i / WIDTH;
    int col = i % WIDTH;
    frame2[row][col] = static_cast<float>(rotated[i]);
  }

  // ——— Compute global optical‐flow (u,v) via Lucas–Kanade ———
  float u = 0.0f, v = 0.0f;
  computeOpticalFlow(frame1, frame2, u, v);

  if ( u == 0 && v == 0 )
  {
    directionWay     = String( "none" );
    pendingDirection = false;
    pendingTimer     = now;
    direction = GESTURE_NONE;
    return;
  }

  // Convert (u,v) to an angle in [0,360)
  float angleRad = atan2(v, u);                // range (–π, +π]
  float angleDeg = angleRad * 180.0f / M_PI;    // → degrees
  if (angleDeg < 0) {
    angleDeg += 360.0f;
  }

  // Map angle to one of eight discrete codes
  const char* dirLabelPtr = nullptr;
  if      (angleDeg < 22.5f || angleDeg >= 337.5f) { dirLabelPtr = "→  (Right)";      direction = GESTURE_RIGHT; }
  else if (angleDeg <  67.5f)                      { dirLabelPtr = "↗  (Up-Right)";   direction = GESTURE_UP_RIGHT; }
  else if (angleDeg < 112.5f)                      { dirLabelPtr = "↑  (Up)";         direction = GESTURE_UP; }
  else if (angleDeg < 157.5f)                      { dirLabelPtr = "↖  (Up-Left)";    direction = GESTURE_UP_LEFT; }
  else if (angleDeg < 202.5f)                      { dirLabelPtr = "←  (Left)";       direction = GESTURE_LEFT; }
  else if (angleDeg < 247.5f)                      { dirLabelPtr = "↙  (Down-Left)";  direction = GESTURE_DOWN_LEFT; }
  else if (angleDeg < 292.5f)                      { dirLabelPtr = "↓  (Down)";       direction = GESTURE_DOWN; }
  else                                             { dirLabelPtr = "↘  (Down-Right)"; direction = GESTURE_DOWN_RIGHT; }

  directionWay     = String(dirLabelPtr);
  pendingDirection = true;
  pendingTimer     = now;

  // (Actual printing of dirLabelPtr is deferred until pendingDecision)
  return;
}
