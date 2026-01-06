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
#include <cfloat>    // for FLT_MAX
#include <Preferences.h>
#include "Logger.h"

// -------- Thresholds in millivolts --------
#define batterysleep   2500
#define batterylow     2900
#define batterymedium  3700
#define batteryfull    4100  // mV at 100% charge

// -------- Battery model --------
// Set to the *actual* cell capacity of your watch battery.
#ifndef BATTERY_CAPACITY_MAH
#define BATTERY_CAPACITY_MAH 450
#endif

extern LOGGER logger;

class Battery {
public:
  Battery();

  void begin();
  void loop();

  /** Returns the last-measured battery voltage in mV */
  uint16_t getVoltage();

  /** Returns true if voltage is below the low threshold */
  bool isBatteryLow();

  /**
   * Returns estimated battery percentage (0–100%)
   * based on linear mapping between batterylow and batteryfull.
   */
  float getBatteryPercent();

  /**
   * Returns a simple level (1=low, 2=medium, 3=high, 4=full-ish) based on thresholds
   */
  int getBatteryLevel();

  String getBatteryStats();

  bool isCharging();

  /**
   * Returns the average *milliwatts* consumed over approximately the last minute.
   * Uses SoC change (from voltage) across the recent window and battery capacity.
   * If there isn't enough data yet, returns 0.
   * By default negative (net charging) values are clamped to 0; pass allowNegative=true to get negatives.
   */
  int getRecentAvgMilliwatts(bool allowNegative = false);

private:
  /** Reads the ADC and updates _voltageMv */
  void readVoltage_();

  /** Convert a millivolt reading to percent (0..100), clamped, no ADC read */
  float percentFromMv_(uint16_t mv) const;

  /** Push a (time,mV) sample into the ring buffer */
  void pushSample_(uint16_t mv, uint32_t t_ms);

  /** Attempt to find a sample ~1 minute ago; returns index or 0xFF if none */
  uint8_t findSampleAboutOneMinuteAgo_(uint32_t now_ms) const;

  // --- existing ---
  uint16_t _voltageMv;
  unsigned long batck;

  unsigned long batstatustime;

  // --- new: lightweight 1‑minute power estimation buffer ---
  struct PwrSample {
    uint32_t t_ms;
    uint16_t mv;
  };

  static const uint8_t kPwrCapacity = 16;      // ~1 minute at 4s cadence
  PwrSample _pwr[kPwrCapacity];
  uint8_t   _pwrHead;                          // points to next write slot
  uint8_t   _pwrCount;                         // how many valid samples
  unsigned long _pwrLastSampleMs;              // last time we sampled
};

#endif // _BATTERY_
