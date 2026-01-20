/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

// SleepService.h
#pragma once
#include <Arduino.h>

class SleepService
{
public:
  SleepService();

  void begin(bool armNow = true);
  void loop();

  // ReflectionsOS.ino uses these
  bool gettingSleepy();          // one-shot at 5 minutes inactivity ONLY
  bool shouldDeepSleep() const;  // true for low battery, user sleep gesture, or 8 minutes inactivity
  String getReasonForSleep() const;

  // Notifications
  void notifyLowBattery();          // deep sleep path reason = low battery
  void notifyUserWantsSleep();      // deep sleep path reason = user gesturing for sleep
  void notifyWatchFaceActivity();   // resets BOTH timers to 0
  void notifyExperienceActivity();  // resets BOTH timers to 0

  // Optional manual reset
  void resetSleep(const char* reason = nullptr);

  void setDebug(bool v) { _debug = v; }

private:
  void setReason_(const __FlashStringHelper* r);

private:
  const uint32_t _startSleepExperienceMs = 5UL * 60UL * 1000UL; // 5 minutes
  const uint32_t _deepSleepMs           = 8UL * 60UL * 1000UL; // 5 + 3

  bool _armed = false;
  bool _debug = false;

  bool _lowBatteryLatched = false;
  bool _userWantsSleep = false;

  uint32_t _sleepyStartMs = 0;
  uint32_t _timeToSleepMs = 0;

  bool _sleepExperienceFired = false;
  bool _sleepExperiencePulse = false;

  String _reason;

  uint32_t _lastDebugMs = 0;
};