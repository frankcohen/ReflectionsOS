/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires ESP32Time library from https://github.com/fbiego/ESP32Time
*/

#ifndef _RealTimeClock_
#define _RealTimeClock_

#include "Haptic.h"
extern Haptic haptic;

#include <Arduino.h>
#include <ESP32Time.h>
#include <time.h>

class RealTimeClock {
public:
  RealTimeClock();

  // Initialize clock behavior.
  // On cold boot/WAKE: load from NVS, else default to 12:00 AM.
  // On deep-sleep wake: do NOT overwrite RTC from NVS (clock continued).
  void begin(int utcOffsetSeconds = 0);

  void loop();

  // Basic accessors (24-hour values from ESP32Time; date ignored)
  int getHour();
  int getMinute();
  int getSecond();

  // 12-hour display string "h:mm" (never returns "--:--")
  String getTime();

  // Manual setting: hour12 is 1-12, ampm: 0=AM, 1=PM
  void setTime(int hour12, int minute, int ampm);

  // Manual setting: set hour+minute, preserve current AM/PM
  void setHourMinute(int hour12, int minute);

  // Epoch helpers (optional use). We still save the clock components to NVS.
  void setEpoch(time_t epoch);
  time_t getEpoch();

  // Save/restore clock components to NVS
  void saveClockToNVS();
  bool loadClockFromNVS();

  // True once we've ensured the RTC has a reasonable clock value
  bool hasValidClock() const { return _hasValidClock; }

  // Optional; used by BoardInitializationUtility
  bool syncWithNTP(const char* ntpServer = "pool.ntp.org", uint32_t timeoutMs = 5000);

private:
  void ensureClockValid_();
  void setDefaultClock_();          // sets RTC to 12:00 AM (dummy date internally)
  void setClock12_(int hour12, int minute, int ampm, int second = 0);

  // Convert current RTC hour -> 12-hour + ampm
  void getHour12AmPm_(int& outHour12, int& outAmPm) const;

private:
  int  _utcOffsetSeconds = 0;       // kept for compatibility; not used for display in 12h mode
  int  _lastBuzzedHour = -1;
  bool _hasValidClock = false;
};

#endif // _RealTimeClock_