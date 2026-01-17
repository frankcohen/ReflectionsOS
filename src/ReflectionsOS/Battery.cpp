/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Battery.h"
#include <esp32-hal-adc.h>

Battery::Battery() {}

void Battery::begin()
{
  pinMode(Battery_Sensor, INPUT);

  // Set attenuation globally (works reliably on core 3.2.0)
  analogSetAttenuation(ADC_11db);

  // Prime/attach the ADC channel for this pin (prevents "not configured as analog channel")
  (void)analogRead(Battery_Sensor);

  _lastSampleMs = millis();
  sample_(); // seed first reading

  Serial.printf("Battery: now=%u mV min=%u mV avg=%u mV\n",
                getVoltageMv(),
                getMinRecentMv(),
                getAvgRecentMv());
  Serial.flush();

  _onBatteryStartMs = _lastSampleMs;
  _onBatteryStarted = true; // best-effort; replace with real VBUS detect later if available
}

void Battery::loop()
{
  const uint32_t now = millis();
  if (now - _lastSampleMs >= kSampleEveryMs) {
    _lastSampleMs = now;
    sample_();
  }
}

void Battery::sample_()
{
  // raw mV at ADC pin
  uint32_t raw = analogReadMilliVolts(Battery_Sensor);
  if (raw > 65535) raw = 65535;
  _rawAdcMv = (uint16_t)raw;

  // scaled to battery terminal mV estimate
  float scaled = (float)_rawAdcMv * BATTERY_ADC_TO_BATT_SCALE;
  if (scaled < 0) scaled = 0;
  if (scaled > 65535) scaled = 65535;
  _mvNow = (uint16_t)(scaled + 0.5f);

  // Push into ring buffer
  const uint32_t t = millis();
  _buf[_head] = { t, _mvNow };
  _head = (_head + 1) % kCap;
  if (_count < kCap) _count++;

  if (!_onBatteryStarted) {
    _onBatteryStartMs = t;
    _onBatteryStarted = true;
  }
}

uint16_t Battery::getVoltageMv() const
{
  return _mvNow;
}

uint16_t Battery::minOverMs_(uint32_t windowMs) const
{
  if (_count == 0) return 0;
  const uint32_t now = millis();
  uint16_t minv = 0xFFFF;

  for (uint16_t i = 0; i < _count; i++) {
    const uint16_t idx = (uint16_t)((_head + kCap - 1 - i) % kCap);
    const Sample& s = _buf[idx];
    const uint32_t age = now - s.t_ms;
    if (age > windowMs) break;
    if (s.mv < minv) minv = s.mv;
  }

  if (minv == 0xFFFF) return _mvNow;
  return minv;
}

uint16_t Battery::avgOverMs_(uint32_t windowMs) const
{
  if (_count == 0) return 0;
  const uint32_t now = millis();
  uint32_t sum = 0;
  uint16_t n = 0;

  for (uint16_t i = 0; i < _count; i++) {
    const uint16_t idx = (uint16_t)((_head + kCap - 1 - i) % kCap);
    const Sample& s = _buf[idx];
    const uint32_t age = now - s.t_ms;
    if (age > windowMs) break;
    sum += s.mv;
    n++;
  }

  if (n == 0) return _mvNow;
  return (uint16_t)((sum + (n / 2)) / n);
}

bool Battery::computeDeltaOverMs_(uint32_t windowMs, int32_t& outDeltaMv, uint32_t& outDtMs) const
{
  outDeltaMv = 0;
  outDtMs = 0;
  if (_count < 2) return false;

  const uint32_t now = millis();
  const uint16_t newestIdx = (uint16_t)((_head + kCap - 1) % kCap);
  const Sample& newest = _buf[newestIdx];

  const Sample* oldest = nullptr;
  for (uint16_t i = 0; i < _count; i++) {
    const uint16_t idx = (uint16_t)((_head + kCap - 1 - i) % kCap);
    const Sample& s = _buf[idx];
    const uint32_t age = now - s.t_ms;
    if (age > windowMs) break;
    oldest = &s;
  }

  if (!oldest) return false;

  outDtMs = (uint32_t)(newest.t_ms - oldest->t_ms);
  if (outDtMs < 1000) return false;

  // Positive = dropping
  outDeltaMv = (int32_t)oldest->mv - (int32_t)newest.mv;
  return true;
}

uint16_t Battery::getMinRecentMv() const
{
  return minOverMs_(kMinWindowMs);
}

uint16_t Battery::getAvgRecentMv() const
{
  return avgOverMs_(kAvgWindowMs);
}

int16_t Battery::getDropRateMvPerMin() const
{
  int32_t dMv = 0;
  uint32_t dtMs = 0;

  if (!computeDeltaOverMs_(kTrendWindowMs, dMv, dtMs)) {
    if (!computeDeltaOverMs_(kAvgWindowMs, dMv, dtMs)) return 0;
  }

  const float minutes = (float)dtMs / 60000.0f;
  if (minutes <= 0.0f) return 0;

  const float rate = (float)dMv / minutes;
  if (rate > 32767.0f) return 32767;
  if (rate < -32768.0f) return -32768;
  return (int16_t)(rate + (rate >= 0 ? 0.5f : -0.5f));
}

uint32_t Battery::getOnBatterySeconds() const
{
  if (!_onBatteryStarted) return 0;
  return (millis() - _onBatteryStartMs) / 1000UL;
}

bool Battery::shouldSleepToProtectRTC() const
{
  // Latch so it doesn't flicker. Once true, it stays true until reboot/deep sleep.
  static bool sLatched = false;

  // Grace period after boot/wake so startup load / early samples don't insta-latch.
  static const uint32_t kGraceMs = 30000; // 30s
  static const uint32_t bootMs = millis();   // ok in Arduino context

  if (sLatched) return true;

  // During grace period, never request protective sleep.
  if (millis() - bootMs < kGraceMs) return false;

  const uint16_t vMin = getMinRecentMv();

  // Require consecutive low detections to avoid one-off dips.
  static uint8_t lowHits = 0;

  if (vMin > 0 && vMin <= BATTERY_SLEEP_MIN_MV) {
    if (lowHits < 255) lowHits++;
  } else {
    lowHits = 0;
  }

  // Latch after N confirmations
  if (lowHits >= 3) {
    sLatched = true;
  }

  return sLatched;
}

uint8_t Battery::getBatteryPercent() const
{
  if (_mvNow <= BATTERY_EMPTY_MV) return 0;
  if (_mvNow >= BATTERY_FULL_MV) return 100;

  const float pct = ((float)(_mvNow - BATTERY_EMPTY_MV) /
                     (float)(BATTERY_FULL_MV - BATTERY_EMPTY_MV)) * 100.0f;

  int p = (int)(pct + 0.5f);
  if (p < 0) p = 0;
  if (p > 100) p = 100;
  return (uint8_t)p;
}

String Battery::getBatteryStats()
{
  // IMPORTANT: Do NOT call sample_() here.
  // Sampling is controlled by Battery::loop() cadence to avoid over-sampling/noise.

  const uint16_t vNow = _mvNow;
  const uint16_t vMin = getMinRecentMv();
  const uint16_t vAvg = getAvgRecentMv();
  const int16_t  drop = getDropRateMvPerMin();
  const uint32_t sec  = getOnBatterySeconds();
  const uint16_t raw  = _rawAdcMv;

  // Two compact lines for your display:
  // Line 1: raw + now + min
  // Line 2: avg + slope + elapsed + sleep flag
  String line1 = " " + String(raw) + " " + String(vNow) + " " + String(vMin);
  String line2 = "  " + String(vAvg) + " " + String(drop) + " " + String(sec/60) + " " +
                 String(shouldSleepToProtectRTC() ? "SLEEP!" : "--");

  return line1 + "\n" + line2;
}

// =====================================================
// Compatibility methods (so the rest of your code builds)
// =====================================================

bool Battery::isBatteryLow()
{
  // Conservative: use sag-based protection signal.
  return shouldSleepToProtectRTC();
}

int Battery::getBatteryLevel()
{
  // Stable leaves: use averaged voltage, not instantaneous sag.
  const uint16_t mv = getAvgRecentMv();

  // 1..4 tiers (distinct thresholds)
  const uint16_t t1 = BATTERY_SLEEP_MIN_MV; // <= 1 leaf
  const uint16_t t2 = BATTERY_WARN_MV;      // <= 2 leaves
  const uint16_t t3 = 4050;                 // <= 3 leaves
  // > t3 => 4 leaves

  if (mv <= t1) return 1;
  if (mv <= t2) return 2;
  if (mv <= t3) return 3;
  return 4;
}