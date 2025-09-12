/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include <Arduino.h>
#include <esp32-hal-adc.h>

#include "Battery.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>

#ifndef Battery_Sensor
#define Battery_Sensor 16  // ensure default if not provided elsewhere
#endif

// ---- Sampling cadence for the power estimator ----
static const uint32_t kPwrSampleEveryMs = 4000;   // take a sample every 4s
static const uint32_t kTargetWindowMs   = 60000;  // ~1 minute window
static const uint32_t kMinWindowMs      = 20000;  // need at least 20s of history

Battery::Battery()
  : _voltageMv(0),
    batck(0),
    _pwrHead(0),
    _pwrCount(0),
    _pwrLastSampleMs(0)
{}

void Battery::begin() 
{
  pinMode(Battery_Sensor, INPUT);
  analogSetPinAttenuation(Battery_Sensor, ADC_11db);

  // initialize timers
  batck = millis();
  _pwrLastSampleMs = batck;

  batstatustime = millis();

  // take an initial reading
  readVoltage_();

  // seed first power sample
  pushSample_(_voltageMv, batck);
}

void Battery::readVoltage_() {
  // Note: analogReadMilliVolts() returns mV according to ESP32 ADC calibration
  // You already applied a scaling factor for your divider & calibration:
  _voltageMv = analogReadMilliVolts(Battery_Sensor) * 5.4014f;
  //Serial.printf("Battery voltage: %u mV\n", _voltageMv);
}

uint16_t Battery::getVoltage() {
  return _voltageMv;
}

bool Battery::isBatteryLow() {
  readVoltage_();
  return _voltageMv < batterysleep;
}

float Battery::getBatteryPercent() 
{
  readVoltage_();
  if (_voltageMv <= batterylow) return 0.0f;
  if (_voltageMv >= batteryfull) return 100.0f;
  return ( ( _voltageMv - static_cast<float>(batterylow) )
       / static_cast<float>(batteryfull - batterylow) )
       * 100.0f;
}

// Returns 1-4 value depending on battery voltage
int Battery::getBatteryLevel() 
{
  readVoltage_();
  if ( _voltageMv < batterysleep ) return 1;
  if ( _voltageMv < batterylow ) return 2;
  if ( _voltageMv < batterymedium ) return 3;
  return 4;
}

String Battery::getBatteryStats() {
  // use your existing legacy readVoltage_()
  readVoltage_();  
  // _voltageMv is already scaled by 5.4014
  int v   = int((_voltageMv + 5) / 10) * 10;         // round to nearest 10 mV
  int pct = int(round(getBatteryPercent()));         // still calls readVoltage_()
  return String(v) + "mV " + String(pct) + "%";
}

// ========== New: power estimation over ~1 minute ==========

float Battery::percentFromMv_(uint16_t mv) const {
  if (mv <= batterylow)  return 0.0f;
  if (mv >= batteryfull) return 100.0f;
  return ( ( mv - static_cast<float>(batterylow) )
       / static_cast<float>(batteryfull - batterylow) )
       * 100.0f;
}

void Battery::pushSample_(uint16_t mv, uint32_t t_ms) {
  _pwr[_pwrHead].t_ms = t_ms;
  _pwr[_pwrHead].mv   = mv;
  _pwrHead = (_pwrHead + 1) % kPwrCapacity;
  if (_pwrCount < kPwrCapacity) {
    _pwrCount++;
  }
}

uint8_t Battery::findSampleAboutOneMinuteAgo_(uint32_t now_ms) const {
  if (_pwrCount == 0) return 0xFF;

  // Iterate oldest -> newest to find the sample with age >= ~1min (or closest older)
  // Compute index of oldest sample
  uint8_t oldestIdx = (_pwrHead + kPwrCapacity - _pwrCount) % kPwrCapacity;

  uint8_t bestIdx = 0xFF;
  uint32_t bestAge = 0;

  for (uint8_t i = 0; i < _pwrCount; ++i) {
    uint8_t idx = (oldestIdx + i) % kPwrCapacity;
    uint32_t age = now_ms - _pwr[idx].t_ms; // unsigned handles wrap safely
    if (age >= kTargetWindowMs) {
      // First sample at or beyond 60s — take it and stop (closest to 60s)
      return idx;
    }
    // Track the oldest (largest age) seen so far (< 60s)
    if (age > bestAge) {
      bestAge = age;
      bestIdx = idx;
    }
  }
  // If nothing >=60s yet, return the oldest we have (may be <60s)
  return bestIdx;
}

int Battery::getRecentAvgMilliwatts(bool allowNegative) {
  if (_pwrCount < 2) return 0;

  const uint32_t now = millis();
  const uint8_t idxThen = findSampleAboutOneMinuteAgo_(now);
  if (idxThen == 0xFF) return 0;

  const PwrSample thenS = _pwr[idxThen];

  // Use the newest sample as "now"; that's always the previous slot (head-1)
  const uint8_t newestIdx = (_pwrHead + kPwrCapacity - 1) % kPwrCapacity;
  const PwrSample nowS = _pwr[newestIdx];

  // Require at least a minimal window to reduce noise
  const float dt_min = float(nowS.t_ms - thenS.t_ms) / 60000.0f;
  if (dt_min <= 0.0f || (nowS.t_ms - thenS.t_ms) < kMinWindowMs) return 0;

  const float pct_then = percentFromMv_(thenS.mv);
  const float pct_now  = percentFromMv_(nowS.mv);

  float delta_pct = pct_then - pct_now;   // positive => net discharge
  if (!allowNegative && delta_pct < 0.0f) delta_pct = 0.0f;

  // Convert % drop over dt to average current
  const float mAh_used = BATTERY_CAPACITY_MAH * (delta_pct / 100.0f);
  const float avg_mA   = mAh_used / dt_min;   // mA

  // Use nominal voltage as the average of then & now
  const float V_nom = ( (thenS.mv + nowS.mv) * 0.5f ) / 1000.0f; // volts

  // Average power in milliwatts
  const float mW = avg_mA * V_nom;  // (mA * V) = mW

  if (!allowNegative && mW < 0.0f) return 0;
  int result = (int)(mW + (mW >= 0 ? 0.5f : -0.5f)); // round to nearest int
  return result;
}

/* Returns true when device is connected to USB and battery charging */

bool Battery::isCharging() {
  if (_pwrCount < 2) return false;

  const uint32_t now = millis();
  const uint8_t idxThen = findSampleAboutOneMinuteAgo_(now);
  if (idxThen == 0xFF) return false;

  const PwrSample thenS = _pwr[idxThen];

  // Use most recent voltage
  const uint8_t newestIdx = (_pwrHead + kPwrCapacity - 1) % kPwrCapacity;
  const PwrSample nowS = _pwr[newestIdx];

  // Require minimal window (e.g. 20s) to reduce false positives
  const uint32_t deltaMs = nowS.t_ms - thenS.t_ms;
  if (deltaMs < kMinWindowMs) return false;

  const int deltaMv = static_cast<int>(nowS.mv) - static_cast<int>(thenS.mv);
  return (deltaMv > 15);  // change threshold (mV) to detect rising voltage
}

void Battery::loop() 
{
  const unsigned long now = millis();

  // Existing 10s check
  if ( now - batck > 10000UL ) {
    batck = now;
    if ( isBatteryLow()) {
      logger.info("Battery low (< " + String(batterylow) + "mV): signalling to sleep");
    }
  }

  if ( now - batstatustime > 5000 )
  {
    batstatustime = now;
    Serial.print( battery.getRecentAvgMilliwatts() );
    Serial.print( " average millivolts used" );

    if ( isCharging() )
    {
      Serial.println( ", battery is charging" );
    }
    else
    {
      Serial.println( " " );
    }
  }

  // Power estimator sampling (every ~4s)
  if ( now - _pwrLastSampleMs >= kPwrSampleEveryMs ) {
    _pwrLastSampleMs = now;
    readVoltage_();
    pushSample_(_voltageMv, now);

    // Trim any samples older than ~75s to bound memory (implicit via ring buffer)
    // No explicit trim needed; ring buffer overwrites oldest.
  }
}
