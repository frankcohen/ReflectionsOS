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

// Thresholds in millivolts
#define batterysleep 2500
#define batterylow   2900
#define batterymedium 3700
#define batteryfull  4100  // mV at 100% charge

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
   * Returns a simple level (1=low, 2=medium, 3=high) based on thresholds
   */
  int getBatteryLevel();

  String getBatteryStats();

private:
  /** Reads the ADC and updates _voltageMv */
  void readVoltage_();

  uint16_t _voltageMv;
  unsigned long batck;
};

#endif // _BATTERY_