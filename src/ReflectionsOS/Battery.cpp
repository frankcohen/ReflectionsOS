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

static void formatMmSs_(uint32_t totalSeconds, char out[6])
{
  uint32_t mm = totalSeconds / 60UL;
  uint32_t ss = totalSeconds % 60UL;
  if (mm > 99) mm = 99;
  snprintf(out, 6, "%02lu:%02lu", (unsigned long)mm, (unsigned long)ss);
}

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

  _bootMs = millis();
  _protectRtcLatched = false;
  _lowHits = 0;
}

void Battery::loop()
{
  const uint32_t now = millis();

  // ---- Sampling cadence ----
  if (now - _lastSampleMs >= kSampleEveryMs) {
    _lastSampleMs = now;
    sample_();
  }

  // ---- Debug heartbeat every 2 minutes ----
  if (now - _lastDebugMs >= kDebugEveryMs) {
    _lastDebugMs = now;

    const uint16_t vNow  = getVoltageMv();
    const uint16_t vMin  = getMinRecentMv();
    const uint16_t vAvg  = getAvgRecentMv();
    const int16_t  drop  = getDropRateMvPerMin();
    const uint32_t onSec = getOnBatterySeconds();
    const uint16_t raw   = _rawAdcMv;

    const int leafs = getBatteryLevel(); // 1..4

    char onBuf[6];
    formatMmSs_(onSec, onBuf);

    char sleepBuf[6] = "--:--";
    if (_sleepCountdownMs != 0xFFFFFFFFUL) {
      const uint32_t sleepSec = _sleepCountdownMs / 1000UL;
      formatMmSs_(sleepSec, sleepBuf);
    }

    Serial.printf(
      "BatteryStats: raw=%u now=%u mV min=%u mV avg=%u mV drop=%d mV/min leafs=%d on=%s sleep_in=%s %s\n",
      raw, vNow, vMin, vAvg, (int)drop, leafs, onBuf, sleepBuf,
      (shouldSleepToProtectRTC() ? "SLEEP!" : "")
    );
    Serial.flush();
  }
}

String Battery::getDischargeOverlayText()
{
  const uint32_t nowMs = millis();
  if (nowMs - _lastOverlayMs < kOverlayEveryMs) return String();
  _lastOverlayMs = nowMs;

  const uint16_t vAvg  = getAvgRecentMv();
  const int      leafs = getBatteryLevel();     // 1..4
  const int16_t  drop  = getDropRateMvPerMin(); // positive = dropping

  // If discharging (drop > 0), estimate minutes to BATTERY_EMPTY_MV.
  if (vAvg <= BATTERY_EMPTY_MV) {
    return " EMPTY " + String(vAvg) + "mV L" + String(leafs);
  }

  if (drop > 0) {
    const uint16_t deltaMv = (uint16_t)(vAvg - BATTERY_EMPTY_MV);
    const uint32_t minsLeft = (deltaMv + (uint16_t)(drop - 1)) / (uint32_t)drop; // ceil divide

    return " " + String(vAvg) + "mV L" + String(leafs) + " " + String(minsLeft) + "m";
  }

  // Not discharging (USB attached / recovery / noise): show level but no estimate.
  return " " + String(vAvg) + "mV L" + String(leafs) + " --m";
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

bool Battery::shouldSleepToProtectRTC()
{
  if (_protectRtcLatched) return true;

  static const uint32_t kGraceMs = 30000; // 30s
  static const uint16_t kPanicMv = 3200;

  const uint16_t vMin = getMinRecentMv();
  if (vMin > 0 && vMin <= kPanicMv) {
    _protectRtcLatched = true;
    return true;
  }

  if (millis() - _bootMs < kGraceMs) return false;

  const uint16_t vAvg = getAvgRecentMv();

  if (vAvg > 0 && vAvg <= BATTERY_SLEEP_MIN_MV) {
    if (_lowHits < 255) _lowHits++;
  } else {
    _lowHits = 0;
  }

  if (_lowHits >= 3) _protectRtcLatched = true;

  return _protectRtcLatched;
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
  const uint16_t vNow = _mvNow;
  const uint16_t vMin = getMinRecentMv();
  const uint16_t vAvg = getAvgRecentMv();
  const int16_t  drop = getDropRateMvPerMin();
  const uint32_t sec  = getOnBatterySeconds();
  const uint16_t raw  = _rawAdcMv;

  String line1 = " " + String(raw) + " " + String(vNow) + " " + String(vMin);
  String line2 = "  " + String(vAvg) + " " + String(drop) + " " + String(sec/60) + " " +
                 String(shouldSleepToProtectRTC() ? "SLEEP!" : "--");

  return line1 + "\n" + line2;
}

bool Battery::isBatteryLow()
{
  return shouldSleepToProtectRTC();
}

int Battery::getBatteryLevel()
{
  const uint16_t mv = getAvgRecentMv();

  const uint16_t emptyMv = BATTERY_EMPTY_MV;
  const uint16_t lowMv   = BATTERY_LOW_MV;
  const uint16_t medMv   = BATTERY_MED_MV;

  if (!(emptyMv <= lowMv && lowMv <= medMv)) {
    return 1;
  }

  if (mv <= emptyMv) return 1;
  if (mv <= lowMv)   return 2;
  if (mv <= medMv)   return 3;
  return 4;
}