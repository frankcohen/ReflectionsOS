/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

// RealTimeClock.cpp
#include "RealTimeClock.h"
#include <Arduino.h>
#include <sys/time.h>
#include <time.h>

// RealTimeClock.cpp
#include "RealTimeClock.h"
#include <Arduino.h>
#include <sys/time.h>
#include <time.h>

RealTimeClock::RealTimeClock() {}

void RealTimeClock::begin() {
  // optional: ensure TZ is set so time APIs work
  setenv("TZ", "GMT0", 1);
  tzset();

  // start the sync state machine
  _state      = State::CheckRTC;
  _lastSaveMs = millis();
}

void RealTimeClock::loop() {
  struct tm tmBuf;

  switch (_state) {
    case State::CheckRTC: {
      // after deep-sleep, time() is preserved—use that
      time_t now = time(nullptr);
      struct tm lt;
      localtime_r(&now, &lt);
      if ((lt.tm_year + 1900) >= MIN_VALID_YEAR) {
        // RTC already valid
        _state = State::Done;
      } else {
        Serial.println("RTC invalid—starting GPS sync…");
        gps.on();
        _gpsStartMs = millis();
        _state      = State::GPSWait;
      }
      break;
    }

    case State::GPSWait:
      gps.loop();
      if (gps.getProcessed() >= GPS_SENTENCE_THRESHOLD ||
          (millis() - _gpsStartMs) >= GPS_TIMEOUT_MS) {
        if (!(gps.getHour() == 0 && gps.getMinute() == 0)) {
          // got GPS fix
          tmBuf.tm_year = gps.getYear()  - 1900;
          tmBuf.tm_mon  = gps.getMonth() - 1;
          tmBuf.tm_mday = gps.getDay();
          tmBuf.tm_hour = (gps.getHour() + timeRegionOffset) % 24;
          tmBuf.tm_min  = gps.getMinute();
          tmBuf.tm_sec  = 0;
          Serial.printf("GPS sync OK: %02d:%02d %02d/%02d/%04d\n",
                        tmBuf.tm_hour, tmBuf.tm_min,
                        tmBuf.tm_mon + 1, tmBuf.tm_mday,
                        tmBuf.tm_year + 1900);
          _candidate = mktime(&tmBuf);
        } else {
          // fallback via NVS or 10:10
          Serial.println("Getting time from NVS or fallback to 10:10 AM");
          time_t saved;
          if (loadFromNVS(saved)) {
            _candidate = saved;
            struct tm lt;
            localtime_r(&saved, &lt);
            Serial.printf("loadFromNVS: %02d:%02d (epoch %lu)\n",
                          lt.tm_hour, lt.tm_min, (unsigned long)saved);
          } else {
            Serial.println("No NVS: fallback 10:10 AM");
            time_t now = time(nullptr);
            localtime_r(&now, &tmBuf);
            tmBuf.tm_hour = 10;
            tmBuf.tm_min  = 10;
            tmBuf.tm_sec  = 0;
            _candidate = mktime(&tmBuf);
          }
        }
        gps.off();
        _state = State::ApplyTime;
      }
      break;

    case State::ApplyTime: {
        applyTimestamp(_candidate, true);
        Serial.println("RTC set and saved to NVS.");

        // confirm
        time_t now2 = time(nullptr);
        struct tm cf;
        localtime_r(&now2, &cf);
        Serial.printf("RTC now %02d:%02d\n", cf.tm_hour, cf.tm_min);

        _state = State::Done;
        break;
      }

    case State::Done:
      // sync finished
      break;
  }

  // save to NVS every 2 minutes
  periodicSave();
}

void RealTimeClock::applyTimestamp(time_t t, bool saveToNVS) {
  struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
  settimeofday(&tv, nullptr);
  if (saveToNVS) {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putULong(NVS_KEY_LAST_TS, (unsigned long)t);
    _prefs.end();
  }
}

bool RealTimeClock::loadFromNVS(time_t &t) {
  _prefs.begin(NVS_NAMESPACE, true);
  unsigned long stored = _prefs.getULong(NVS_KEY_LAST_TS, 0);
  _prefs.end();
  if (stored > 0) {
    t = (time_t)stored;
    return true;
  }
  return false;
}

void RealTimeClock::periodicSave() {
  unsigned long nowMs = millis();
  if (nowMs - _lastSaveMs >= NVS_SAVE_INTERVAL_MS) {
    // debug before
    _prefs.begin(NVS_NAMESPACE, true);
    unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
    _prefs.end();
    {
      struct tm bt;
      time_t be = before;
      localtime_r(&be, &bt);
      Serial.printf("Periodic save: before NVS = %02d:%02d (epoch %lu)\n",
                    bt.tm_hour, bt.tm_min, before);
    }

    // write current
    time_t now = time(nullptr);
    struct tm nm;
    localtime_r(&now, &nm);
    time_t t = mktime(&nm);
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putULong(NVS_KEY_LAST_TS, (unsigned long)t);
    _prefs.end();

    // debug after
    _prefs.begin(NVS_NAMESPACE, true);
    unsigned long after = _prefs.getULong(NVS_KEY_LAST_TS, 0);
    _prefs.end();
    {
      struct tm at;
      time_t ae = after;
      localtime_r(&ae, &at);
      Serial.printf("Periodic save: after  NVS = %02d:%02d (epoch %lu)\n",
                    at.tm_hour, at.tm_min, after);
    }

    _lastSaveMs = nowMs;
  }
}

int RealTimeClock::getHour() {
  time_t now = time(nullptr);
  struct tm tmNow;
  localtime_r(&now, &tmNow);
  return tmNow.tm_hour;
}

int RealTimeClock::getMinute() {
  time_t now = time(nullptr);
  struct tm tmNow;
  localtime_r(&now, &tmNow);
  return tmNow.tm_min;
}

void RealTimeClock::setTime(int hour, int minute, int ampm) {
  time_t now = time(nullptr);
  struct tm tm0;
  localtime_r(&now, &tm0);
  tm0.tm_hour = hour;
  tm0.tm_min  = minute;
  tm0.tm_sec  = 0;
  time_t t = mktime(&tm0);
  applyTimestamp(t, true);
}

String RealTimeClock::getTime() {
  // always return the current RTC time (ensuring it's initialized)
  time_t now = time(nullptr);
  struct tm tm0;
  localtime_r(&now, &tm0);
  if ((tm0.tm_year + 1900) < MIN_VALID_YEAR) {
    // if still uninitialized, fallback to NVS or 10:10
    time_t saved;
    if (loadFromNVS(saved)) {
      applyTimestamp(saved, false);
      now = saved;
      localtime_r(&now, &tm0);
    } else {
      tm0.tm_hour = 10; tm0.tm_min = 10; tm0.tm_sec = 0;
      time_t fb = mktime(&tm0);
      applyTimestamp(fb, true);
      now = fb;
      localtime_r(&now, &tm0);
    }
  }

  char buf[6];
  snprintf(buf, sizeof(buf), "%d:%02d", tm0.tm_hour, tm0.tm_min);  // no leading zero on hour
  return String(buf);
}

