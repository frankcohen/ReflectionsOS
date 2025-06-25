/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _RealTimeClock_
#define _RealTimeClock_

#include "config.h"
#include "secrets.h"
#include "Arduino.h"
#include "Logger.h"
#include "time.h"
#include "gps.h"
#include <Preferences.h>

// Configuration constants
#define MIN_VALID_YEAR         2023
#define GPS_SENTENCE_THRESHOLD 1000
#define GPS_TIMEOUT_MS         5000

#define NVS_NAMESPACE          "rtc"
#define NVS_KEY_LAST_TS        "last_ts"
#define NVS_SAVE_INTERVAL_MS   120000  // 2 minutes

// these must be defined elsewhere in your project:
extern LOGGER       logger;           // your serial‐logging object
extern GPS          gps;              // your GPS interface

class RealTimeClock {
public:
  RealTimeClock();
  void begin();    // kick off the state machine
  void loop();     // drive the state machine
  int  getHour();  
  int  getMinute();
  void setTime(int hour, int minute, int ampm);

  // returns "HH:MM" from RTC if valid, otherwise tries GPS,
  // otherwise returns "10:10" and re-triggers begin()
  String getTime();

private:
  enum class State {
    CheckRTC,    // see if RTC already has a valid time
    GPSWait,     // GPS warming up / collecting sentences
    ApplyTime,   // set the RTC from either GPS or fallback
    Done         // nothing more to do
  };

  void applyTimestamp(time_t t, bool saveToNVS = true);
  bool loadFromNVS(time_t &t);
  void periodicSave();

  State    _state       = State::CheckRTC;
  bool     _rtcValid    = false;
  bool     _gpsValid    = false;
  time_t   _candidate;       // unix timestamp we’ll apply
  unsigned long _gpsStartMs = 0;
  unsigned long _lastSaveMs  = 0;  
  Preferences   _prefs;

};

#endif // _icons_
