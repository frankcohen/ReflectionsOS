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

  // if we woke from a deep-sleep
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  if ( cause == ESP_SLEEP_WAKEUP_EXT1) 
  {
    time_t confirm = time(nullptr);
    struct tm lt;
    localtime_r(&confirm, &lt);
    Serial.printf("RTC woke from deep sleep: %d %02d:%02d\n", lt.tm_year + 1900, lt.tm_hour, lt.tm_min);
    _lastSaveMs = millis();
    _state      = State::Done;
    return;
  }

  // otherwise, proceed with normal sync
  _state      = State::CheckRTC;
  _lastSaveMs = millis();
}

void RealTimeClock::loop() 
{
  // periodic NVS save every 1 minutes
  periodicSave();

  struct tm tmBuf;

  switch (_state) {
    case State::CheckRTC: {
      time_t now = time(nullptr);
      struct tm lt;
      localtime_r(&now, &lt);
      if ( ( (lt.tm_year + 1900) >= MIN_VALID_YEAR ) && ( tmBuf.tm_hour > 0 ) ) 
      {
        Serial.printf("RTC valid: %d %02d:%02d\n", lt.tm_year + 1900, tmBuf.tm_hour, tmBuf.tm_min);
        _state = State::Done;
      } 
      else 
      {
        Serial.println("RTC invalid, trying GPS sync");
        gps.on();
        _gpsStartMs = millis();
        _state      = State::GPSWait;
      }
      break;
    }

    case State::GPSWait:
      gps.loop();
      if (gps.getProcessed() >= GPS_SENTENCE_THRESHOLD ||
          (millis() - _gpsStartMs) >= GPS_TIMEOUT_MS) 
      {
        if ( ( ! ( gps.getHour() == 0 && gps.getMinute() == 0 ) ) && gps.getSatellites() > 2 ) 
        {
          tmBuf.tm_year = gps.getYear()  - 1900;
          tmBuf.tm_mon  = gps.getMonth() - 1;
          tmBuf.tm_mday = gps.getDay();
          tmBuf.tm_hour = (gps.getHour() + timeRegionOffset) % 24;
          tmBuf.tm_min  = gps.getMinute();
          tmBuf.tm_sec  = 0;
          Serial.printf("GPS sync OK: %02d:%02d\n", tmBuf.tm_hour, tmBuf.tm_min);
          _candidate = mktime(&tmBuf);
        } 
        else 
        {
          Serial.println("Getting time from NVS or fallback to 10:10 AM Jan 1 2025");
          time_t saved;
          if (loadFromNVS(saved)) 
          {
            _candidate = saved;
            struct tm lt;
            localtime_r(&saved, &lt);
            Serial.printf("Loaded from NVS: %02d:%02d\n", lt.tm_hour, lt.tm_min);
          } 
          else           
          {
            Serial.println("No NVS: fallback 10:10 AM Jan 1 2025");
            time_t today = time(nullptr);

            memset(&tmBuf, 0, sizeof(tmBuf));  // clear everything
            tmBuf.tm_year = 2025;     // e.g. 2025 → 125
            tmBuf.tm_mon  =  0;       // January (0-based)
            tmBuf.tm_mday =  1;       // 1st of the month
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
  if (nowMs - _lastSaveMs < NVS_SAVE_INTERVAL_MS ) return;

  // 1) Read & fix RTC as before…
  time_t now = time(nullptr);
  struct tm tmNow;
  localtime_r(&now, &tmNow);

  time_t writeEpoch = now;
  if ((tmNow.tm_year + 1900) < MIN_VALID_YEAR) {
    Serial.println("PeriodicSave: RTC invalid, fixing date to Jan 1 2025");
    tmNow.tm_year = 2025 - 1900;
    tmNow.tm_mon  = 0;
    tmNow.tm_mday = 1;
    writeEpoch = mktime(&tmNow);
    // update the system clock right away
    timeval tv{ .tv_sec = writeEpoch, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
  }

  Serial.printf(
    "PeriodicSave: RTC now = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
    tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday,
    tmNow.tm_hour,    tmNow.tm_min,
    (unsigned long)writeEpoch
  );

  // 2) Read & show the old NVS value...
  _prefs.begin(NVS_NAMESPACE, true);
  unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
  _prefs.end();
  if (before) {
    time_t bsec = (time_t)before;             // << convert to time_t
    struct tm tmBefore;
    localtime_r(&bsec, &tmBefore);
    if ((tmBefore.tm_year + 1900) < MIN_VALID_YEAR) {
      tmBefore.tm_year = 2025 - 1900;
      tmBefore.tm_mon  = 0;
      tmBefore.tm_mday = 1;
    }
    Serial.printf(
      "PeriodicSave: NVS before = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      tmBefore.tm_year + 1900, tmBefore.tm_mon + 1,
      tmBefore.tm_mday,      tmBefore.tm_hour,
      tmBefore.tm_min,       before
    );
  } else {
    Serial.println("PeriodicSave: NVS before = <none>");
  }

  // 3) Write the corrected epoch into NVS
  _prefs.begin(NVS_NAMESPACE, false);
  _prefs.putULong(NVS_KEY_LAST_TS, (unsigned long)writeEpoch);
  _prefs.end();

  // 4) Read & show the new NVS value (properly converted)
  _prefs.begin(NVS_NAMESPACE, true);
  unsigned long after = _prefs.getULong(NVS_KEY_LAST_TS, 0);
  _prefs.end();
  {
    time_t asec = (time_t)after;             // << convert to time_t
    struct tm tmAfter;
    localtime_r(&asec, &tmAfter);
    if ((tmAfter.tm_year + 1900) < MIN_VALID_YEAR) {
      tmAfter.tm_year = 2025 - 1900;
      tmAfter.tm_mon  = 0;
      tmAfter.tm_mday = 1;
    }
    Serial.printf(
      "PeriodicSave: NVS after  = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      tmAfter.tm_year + 1900, tmAfter.tm_mon + 1,
      tmAfter.tm_mday,       tmAfter.tm_hour,
      tmAfter.tm_min,        after
    );
  }

  _lastSaveMs = nowMs;
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

  int myhour = tmBuf.tm_hour;
  int myminute = tmBuf.tm_min;
  if ( myhour < 1 ) myhour = 1;
  if ( myhour > 12 ) myhour = 12;
  if ( myminute < 1 ) myminute = 1;
  if ( myminute > 59 ) myminute = 59;

  char buf[6];
  snprintf( buf, sizeof(buf), "%d:%02d", myhour, myminute );
  return String(buf);
}

// static helper to check for a “reasonable” time
static bool timeIsValid(time_t t) {
  struct tm tmNow;
  localtime_r(&t, &tmNow);
  return (tmNow.tm_year + 1900) >= MIN_VALID_YEAR;
}

bool RealTimeClock::syncWithNTP(const char* ntpServer, uint32_t timeoutMs) {
  Serial.printf("Starting NTP sync with %s (timeout %lums)\n", ntpServer, timeoutMs);

  // zero UTC offset, no DST
  configTime(0, 0, ntpServer);

  time_t now;
  uint32_t start = millis();
  // wait up to timeoutMs for the SNTP client to set the time
  while ((now = time(nullptr)), !timeIsValid(now)) {
    if (millis() - start > timeoutMs) {
      Serial.println("NTP sync failed: timeout");
      return false;
    }
    delay(100);
  }

  // print the new time
  Serial.printf("NTP time acquired: %s", ctime(&now));

  // apply to RTC and save to NVS
  applyTimestamp(now, true);
  Serial.println("RTC updated from NTP and saved to NVS.");
  return true;
}

