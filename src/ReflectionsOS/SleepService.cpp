/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

// SleepService.cpp
#include "SleepService.h"

static const __FlashStringHelper* R_INACTIVITY = F("Going to sleep for inactivity");
static const __FlashStringHelper* R_LOW_BAT    = F("Going to sleep for low battery");
static const __FlashStringHelper* R_GESTURE    = F("Going to sleep from the user gesturing for sleep");

SleepService::SleepService() {}

void SleepService::begin(bool armNow)
{
  _armed = armNow;

  _lowBatteryLatched = false;
  _userWantsSleep = false;

  _sleepyStartMs = millis();
  _timeToSleepMs = 0;

  _sleepExperienceFired = false;
  _sleepExperiencePulse = false;

  _reason = "";
}

void SleepService::setReason_(const __FlashStringHelper* r)
{
  const String s = String(r);

  // Priority: low battery > user gesture > inactivity
  if (_reason.length() == 0) { _reason = s; return; }
  if (_reason == String(R_LOW_BAT)) return;
  if (s == String(R_LOW_BAT)) { _reason = s; return; }

  if (_reason == String(R_GESTURE)) return;
  if (s == String(R_GESTURE)) { _reason = s; return; }
}

void SleepService::resetSleep(const char* reason)
{
  const uint32_t now = millis();

  _sleepyStartMs = now;
  _timeToSleepMs = 0;

  // Reset BOTH timers:
  _sleepExperienceFired = false;
  _sleepExperiencePulse = false;

  // Activity cancels user-sleep request; battery latch is authoritative
  _userWantsSleep = false;

  if (!_lowBatteryLatched) _reason = "";

  if (_debug && reason) {
    Serial.printf("SleepService: resetSleep(%s) at %lu\n", reason, (unsigned long)now);
  }
}

void SleepService::notifyWatchFaceActivity()
{
  resetSleep("watchfaceActivity");
}

void SleepService::notifyExperienceActivity()
{
  // No throttling needed; just resets the clocks.
  resetSleep("experienceActivity");
}

void SleepService::notifyLowBattery()
{
  _lowBatteryLatched = true;
  setReason_(R_LOW_BAT);
  if (_debug) Serial.println("SleepService: low battery notified");
}

void SleepService::notifyUserWantsSleep()
{
  _userWantsSleep = true;
  setReason_(R_GESTURE);
  if (_debug) Serial.println("SleepService: user wants sleep (gesture)");
}

void SleepService::loop()
{
  if (!_armed) return;

  // If battery is low or user requested sleep, ReflectionsOS.ino should go to the deep-sleep path.
  // gettingSleepy() should NOT be triggered by these.
  if (_lowBatteryLatched) return;
  if (_userWantsSleep) return;

  const uint32_t now = millis();
  _timeToSleepMs = now - _sleepyStartMs;

  // One-shot at 5 minutes => start Sleep experience (inactivity only)
  if (!_sleepExperienceFired && _timeToSleepMs >= _startSleepExperienceMs) {
    _sleepExperienceFired = true;
    _sleepExperiencePulse = true;
    setReason_(R_INACTIVITY);
  }

  // Ensure reason is correct when deep sleep is due to inactivity
  if (_timeToSleepMs >= _deepSleepMs) {
    setReason_(R_INACTIVITY);
  }

  if (_debug && (now - _lastDebugMs >= 1000)) {
    _lastDebugMs = now;
    Serial.printf("SleepService: timeToSleep=%lu ms fired=%d\n",
      (unsigned long)_timeToSleepMs, (int)_sleepExperienceFired);
  }
}

bool SleepService::gettingSleepy()
{
  // ONLY inactivity 5-minute one-shot
  if (_sleepExperiencePulse) {
    _sleepExperiencePulse = false;
    return true;
  }
  return false;
}

bool SleepService::shouldDeepSleep() const
{
  // Deep sleep reasons:
  if (_lowBatteryLatched) return true;
  if (_userWantsSleep) return true;

  // Inactivity deep sleep at 8 minutes:
  return (_timeToSleepMs >= _deepSleepMs);
}

String SleepService::getReasonForSleep() const
{
  if (_reason.length() == 0) return String(R_INACTIVITY);
  return _reason;
}