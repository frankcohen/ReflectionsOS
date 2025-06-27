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

RealTimeClock::RealTimeClock() {}

void RealTimeClock::begin() {
  // set timezone so time APIs work
  setenv("TZ", "GMT0", 1);
  tzset();

  // if we woke from a deep-sleep timer, restore RTC from NVS
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_TIMER) {
    time_t saved;
    if (loadFromNVS(saved)) {
      applyTimestamp(saved, false);
      Serial.println("Restored RTC from NVS after deep-sleep wake");
      _state      = State::Done;
      _lastSaveMs = millis();
      return;
    }
  }

  // otherwise, proceed with normal sync
  _state      = State::CheckRTC;
  _lastSaveMs = millis();
}

void RealTimeClock::loop() 
{
  // periodic NVS save every 2 minutes
  periodicSave();

  struct tm tmBuf;

  switch (_state) {
    case State::CheckRTC: {
      time_t now = time(nullptr);
      struct tm lt;
      localtime_r(&now, &lt);
      Serial.print("RTC year: ");
      Serial.println(lt.tm_year + 1900);
      if ((lt.tm_year + 1900) >= MIN_VALID_YEAR) {
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
          tmBuf.tm_year = gps.getYear()  - 1900;
          tmBuf.tm_mon  = gps.getMonth() - 1;
          tmBuf.tm_mday = gps.getDay();
          tmBuf.tm_hour = (gps.getHour() + timeRegionOffset) % 24;
          tmBuf.tm_min  = gps.getMinute();
          tmBuf.tm_sec  = 0;
          Serial.printf("GPS sync OK: %02d:%02d\n", tmBuf.tm_hour, tmBuf.tm_min);
          _candidate = mktime(&tmBuf);
        } else {
          Serial.println("Getting time from NVS or fallback to 10:10 AM");
          time_t saved;
          if (loadFromNVS(saved)) {
            _candidate = saved;
            struct tm lt;
            localtime_r(&saved, &lt);
            Serial.printf("Loaded from NVS: %02d:%02d\n", lt.tm_hour, lt.tm_min);
          } else {
            Serial.println("No NVS: fallback 10:10 AM");
            time_t today = time(nullptr);

            memset(&tmBuf, 0, sizeof(tmBuf));  // clear everything
            tmBuf.tm_year = MIN_VALID_YEAR - 1900;  // e.g. 2025 → 125
            tmBuf.tm_mon  =  0;                   // January (0-based)
            tmBuf.tm_mday =  1;                   // 1st of the month
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
      time_t confirm = time(nullptr);
      struct tm cf;
      localtime_r(&confirm, &cf);
      Serial.printf("RTC now %02d:%02d %04d\n", cf.tm_hour, cf.tm_min, cf.tm_year);
      _state = State::Done;
      break;
    }

    case State::Done:
      break;
  }
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
    // 1) Read current RTC time
    time_t now = time(nullptr);
    struct tm tmNow;
    localtime_r(&now, &tmNow);
    Serial.printf(
      "PeriodicSave: RTC now = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      tmNow.tm_year + 1900,
      tmNow.tm_mon  + 1,
      tmNow.tm_mday,
      tmNow.tm_hour,
      tmNow.tm_min,
      (unsigned long)now
    );

    // 2) Read existing NVS stored time
    _prefs.begin(NVS_NAMESPACE, true);
    unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
    _prefs.end();
    if (before > 0) {
      time_t bsec = (time_t)before;
      struct tm tmBefore;
      localtime_r(&bsec, &tmBefore);

      Serial.printf(
        "PeriodicSave: NVS before = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
        tmBefore.tm_year + 1900,
        tmBefore.tm_mon  + 1,
        tmBefore.tm_mday,
        tmBefore.tm_hour,
        tmBefore.tm_min,
        before
      );
    } else {
      Serial.println("PeriodicSave: NVS before = <none>");
    }

    // 3) Write current RTC into NVS
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putULong(NVS_KEY_LAST_TS, (unsigned long)now);
    _prefs.end();

    // 4) Read back and display new NVS value
    _prefs.begin(NVS_NAMESPACE, true);
    unsigned long after = _prefs.getULong(NVS_KEY_LAST_TS, 0);
    _prefs.end();
    time_t asec = (time_t)after;
    struct tm tmAfter;
    localtime_r(&asec, &tmAfter);
    Serial.printf(
      "PeriodicSave: NVS after  = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      tmAfter.tm_year + 1900,
      tmAfter.tm_mon  + 1,
      tmAfter.tm_mday,
      tmAfter.tm_hour,
      tmAfter.tm_min,
      after
    );

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
  applyTimestamp(mktime(&tmBuf), true);
}

String RealTimeClock::getTime() {
  time_t now = time(nullptr);
  struct tm tmBuf;
  localtime_r(&now, &tmBuf);

  if ((tmBuf.tm_year + 1900) < MIN_VALID_YEAR) {
    time_t saved;
    if (loadFromNVS(saved)) {
      applyTimestamp(saved, false);
      localtime_r(&saved, &tmBuf);
    } else {
      tmBuf.tm_hour = 10;
      tmBuf.tm_min  = 10;
      tmBuf.tm_sec  = 0;
      applyTimestamp(mktime(&tmBuf), true);
    }
  }

  char buf[6];
  snprintf(buf, sizeof(buf), "%d:%02d", tmBuf.tm_hour, tmBuf.tm_min);
  return String(buf);
}


