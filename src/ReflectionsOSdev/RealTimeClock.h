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

#include "Arduino.h"
#include <ESP32Time.h>
#include <time.h>

// Configuration constants
#define MIN_VALID_YEAR 2023

class RealTimeClock {
public:
  RealTimeClock();
  
  // Configure RTC behavior (timezone offset, logging, etc.)
  void begin(int utcOffsetSeconds = -28800); // default PST (UTC-8)
  void loop();

  // Basic accessors
  int  getHour();
  int  getMinute();
  int  getSecond();

  // Format time as "HH:MM" (24-hour). If not valid, returns "--:--"
  String getTime();

  // Validity check (simple, based on year)
  bool isValid();

  // Manual setting (hour: 1-12, ampm: 0=AM, 1=PM)
  void setTime(int hour12, int minute, int ampm);

  void setHourMinute(int hour12, int minute);

  // For BoardInitializationUtility: set from epoch or struct tm
  void setEpoch(time_t epoch);
  time_t getEpoch();

  void setTimeStruct(const struct tm& t);

  /**
     * @brief Synchronize RTC from an NTP server.
     * @param ntpServer  Hostname of NTP server (default "pool.ntp.org")
     * @param timeoutMs  How long (ms) to wait for a valid time (default 5 000)
     * @return true if sync succeeded, false on timeout
  */
  bool syncWithNTP(const char* ntpServer = "pool.ntp.org", uint32_t timeoutMs = 5000);

private:
  int _utcOffsetSeconds = -28800;

  int _lastBuzzedHour = -1;  
};

#endif // _RealTimeClock_