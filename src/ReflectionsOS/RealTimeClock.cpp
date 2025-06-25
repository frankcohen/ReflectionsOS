/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "RealTimeClock.h"

#include <Arduino.h>
#include <sys/time.h>  // settimeofday()
#include <time.h>

RealTimeClock::RealTimeClock() {}

void RealTimeClock::begin() {
  // initialize timezone (optional)
  setenv("TZ", "GMT0", 1);
  tzset();

  // start sync state machine
  _state      = State::CheckRTC;

  // initialize timer for periodic saves
  _lastSaveMs = millis();
}

void RealTimeClock::loop() {
  struct tm tmBuf;

  // Drive the sync state machine
  switch (_state) {
    case State::CheckRTC:
      if (getLocalTime(&tmBuf) && (tmBuf.tm_year + 1900) >= MIN_VALID_YEAR) {
        _state = State::Done;
      } else {
        Serial.println("RTC invalid—starting GPS sync…");
        gps.on();
        _gpsStartMs = millis();
        _state      = State::GPSWait;
      }
      break;

    case State::GPSWait:
      gps.loop();
      if (gps.getProcessed() >= GPS_SENTENCE_THRESHOLD ||
          (millis() - _gpsStartMs) >= GPS_TIMEOUT_MS) {
        if (!(gps.getHour() == 0 && gps.getMinute() == 0)) {
          // Use GPS time
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
          // GPS not found: try NVS
          Serial.println("Getting time from NVS or fallback to 10:10 AM");
          time_t saved;
          if (loadFromNVS(saved)) {
            _candidate = saved;
            struct tm lt;
            localtime_r(&saved, &lt);
            Serial.printf("loadFromNVS: stored epoch %lu → %02d:%02d\n",
                          (unsigned long)saved, lt.tm_hour, lt.tm_min);
          } else {
            Serial.println("No NVS: fallback 10:10");
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

        // immediate confirmation
        time_t t2 = time(nullptr);
        struct tm cf;
        localtime_r(&t2, &cf);
        Serial.printf("RTC now %02d:%02d\n", cf.tm_hour, cf.tm_min);

        _state = State::Done;
        break;
      }

    case State::Done:
      // nothing more for sync
      break;
  }

  // Periodically save current RTC to NVS every 2 minutes
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
    // show stored time before saving
    _prefs.begin(NVS_NAMESPACE, true);
    unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
    _prefs.end();
    {
      struct tm bt;
      time_t bt_epoch = before;
      localtime_r(&bt_epoch, &bt);
      // Serial.printf("Periodic save: before NVS = %02d:%02d (epoch %lu)\n", bt.tm_hour, bt.tm_min, before);
    }

    // write current RTC
    time_t now = time(nullptr);
    struct tm tmNow;
    localtime_r(&now, &tmNow);
    time_t t = mktime(&tmNow);
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putULong(NVS_KEY_LAST_TS, (unsigned long)t);
    _prefs.end();

    // show stored time after saving
    _prefs.begin(NVS_NAMESPACE, true);
    unsigned long after = _prefs.getULong(NVS_KEY_LAST_TS, 0);
    _prefs.end();
    {
      struct tm at;
      time_t at_epoch = after;
      localtime_r(&at_epoch, &at);
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
  struct tm tmBuf;
  localtime_r(&now, &tmBuf);
  tmBuf.tm_hour = hour;
  tmBuf.tm_min  = minute;
  tmBuf.tm_sec  = 0;
  time_t t = mktime(&tmBuf);
  applyTimestamp(t, true);
}

String RealTimeClock::getTime() {
  // Return current RTC time, or restore/fallback if invalid
  time_t now = time(nullptr);
  struct tm tmBuf;
  localtime_r(&now, &tmBuf);

  if ((tmBuf.tm_year + 1900) < MIN_VALID_YEAR) {
    // RTC uninitialized: try NVS
    time_t saved;
    if (loadFromNVS(saved)) {
      applyTimestamp(saved, false);
      now = saved;
      localtime_r(&now, &tmBuf);
    } else {
      // no stored time: fallback to 10:10 today
      time_t tday = time(nullptr);
      localtime_r(&tday, &tmBuf);
      tmBuf.tm_hour = 10;
      tmBuf.tm_min  = 10;
      tmBuf.tm_sec  = 0;
      time_t fallback = mktime(&tmBuf);
      applyTimestamp(fallback, true);
      now = fallback;
      localtime_r(&now, &tmBuf);
    }
  }

  char buf[6];
  snprintf(buf, sizeof(buf), "%d:%02d", tmBuf.tm_hour, tmBuf.tm_min);
  return String(buf);
}


