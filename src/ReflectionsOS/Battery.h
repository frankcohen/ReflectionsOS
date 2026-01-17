/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _BATTERY_
#define _BATTERY_

#include <Arduino.h>
#include "Logger.h"

extern LOGGER logger;

// ===============================
// Battery safety thresholds (mV)
// ===============================
// MUST sleep if recent minimum voltage falls below this.
// Start conservative; tune after you observe sag.
#ifndef BATTERY_SLEEP_MIN_MV
#define BATTERY_SLEEP_MIN_MV 3300
#endif

// Optional: used for "battery low" UI tiers (leaves)
#ifndef BATTERY_WARN_MV
#define BATTERY_WARN_MV 3600
#endif

// Percent UI (not safety)
#ifndef BATTERY_FULL_MV
#define BATTERY_FULL_MV 4200
#endif

#ifndef BATTERY_EMPTY_MV
#define BATTERY_EMPTY_MV 3300
#endif

// ===============================
// ADC scaling
// ===============================
#ifndef BATTERY_ADC_TO_BATT_SCALE
#define BATTERY_ADC_TO_BATT_SCALE 5.4014f
#endif

#ifndef Battery_Sensor
#define Battery_Sensor 16
#endif

class Battery {
public:
  Battery();

  void begin();
  void loop();

  // --- Core values (cached) ---
  uint16_t getVoltageMv() const;          // last sampled battery voltage (mV)
  uint16_t getMinRecentMv() const;        // min over ~20s
  uint16_t getAvgRecentMv() const;        // avg over ~60s
  int16_t  getDropRateMvPerMin() const;   // positive = dropping
  uint32_t getOnBatterySeconds() const;

  bool shouldSleepToProtectRTC() const;

  // Two-line display string for video.paintText() (split on '\n')
  String getBatteryStats();

  // UI-only percent (not safety)
  uint8_t getBatteryPercent() const;

  // ==========================================
  // Compatibility methods used by your codebase
  // ==========================================
  /** Legacy: returns true when we should sleep to protect RTC (conservative). */
  bool isBatteryLow();

  /** Legacy: returns 1..4 "leaf level" (stable, based on avg voltage). */
  int getBatteryLevel();

  /** Legacy: old name used in your code (returns cached voltage). */
  uint16_t getVoltage() { return getVoltageMv(); }

private:
  void sample_();

  uint16_t minOverMs_(uint32_t windowMs) const;
  uint16_t avgOverMs_(uint32_t windowMs) const;
  bool     computeDeltaOverMs_(uint32_t windowMs, int32_t& outDeltaMv, uint32_t& outDtMs) const;

private:
  uint16_t _mvNow = 0;
  uint16_t _rawAdcMv = 0;

  uint32_t _onBatteryStartMs = 0;
  bool     _onBatteryStarted = false;

  uint32_t _lastSampleMs = 0;

  struct Sample {
    uint32_t t_ms;
    uint16_t mv;
  };

  static const uint16_t kCap = 180; // ~6 minutes at 2s cadence
  Sample   _buf[kCap];
  uint16_t _head = 0;
  uint16_t _count = 0;

  static const uint32_t kSampleEveryMs = 1000;
  static const uint32_t kMinWindowMs   = 30000;
  static const uint32_t kAvgWindowMs   = 60000;
  static const uint32_t kTrendWindowMs = 300000;
};

#endif // _BATTERY_
