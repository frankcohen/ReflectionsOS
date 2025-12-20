/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "RealTimeClock.h"

// Global RTC instance used by ESP32Time
ESP32Time rtc(0);

RealTimeClock::RealTimeClock() {}

void RealTimeClock::begin(int utcOffsetSeconds) {
  _utcOffsetSeconds = utcOffsetSeconds;

  // ESP32Time uses rtc.offset (seconds)
  rtc.offset = _utcOffsetSeconds;

  Serial.printf(
    "RTC: %04d-%02d-%02d %02d:%02d:%02d (epoch %lu) offset %d\n",
    rtc.getYear(), rtc.getMonth(), rtc.getDay(),
    rtc.getHour(), rtc.getMinute(), rtc.getSecond(),
    (unsigned long)rtc.getEpoch(),
    _utcOffsetSeconds
  );
}

bool RealTimeClock::isValid() {
  return rtc.getYear() >= MIN_VALID_YEAR;
}

int RealTimeClock::getHour() {
  return rtc.getHour();
}

int RealTimeClock::getMinute() {
  return rtc.getMinute();
}

int RealTimeClock::getSecond() {
  return rtc.getSecond();
}

String RealTimeClock::getTime() {
  if (!isValid()) {
    return String("--:--");
  }

  char buf[6];
  snprintf(buf, sizeof(buf), "%2d:%02d", rtc.getHour(), rtc.getMinute());
  return String(buf);
}

// Manual setting: hour12 is 1-12, ampm: 0=AM, 1=PM
void RealTimeClock::setTime(int hour12, int minute, int ampm) {
  // Normalize inputs
  if (hour12 < 1)  hour12 = 1;
  if (hour12 > 12) hour12 = 12;
  if (minute < 0)  minute = 0;
  if (minute > 59) minute = 59;
  ampm = (ampm != 0) ? 1 : 0;

  // Convert to 24-hour
  int hour24 = hour12 % 12;          // 12 -> 0
  if (ampm == 1) hour24 += 12;       // PM adds 12

  // Keep the existing date if RTC is valid, otherwise choose a sane default date
  int year  = isValid() ? rtc.getYear()  : 2025;
  int month = isValid() ? rtc.getMonth() : 1;
  int day   = isValid() ? rtc.getDay()   : 1;

  rtc.setTime(0, minute, hour24, day, month, year);

  Serial.printf(
    "RTC manually set to: %04d-%02d-%02d %02d:%02d:%02d (epoch %lu)\n",
    rtc.getYear(), rtc.getMonth(), rtc.getDay(),
    rtc.getHour(), rtc.getMinute(), rtc.getSecond(),
    (unsigned long)rtc.getEpoch()
  );
}

void RealTimeClock::setEpoch(time_t epoch) {
  rtc.setTime(epoch);

  Serial.printf(
    "RTC setEpoch: %04d-%02d-%02d %02d:%02d:%02d (epoch %lu)\n",
    rtc.getYear(), rtc.getMonth(), rtc.getDay(),
    rtc.getHour(), rtc.getMinute(), rtc.getSecond(),
    (unsigned long)rtc.getEpoch()
  );
}

time_t RealTimeClock::getEpoch() {
  return (time_t)rtc.getEpoch();
}

void RealTimeClock::setHourMinute(int hour12, int minute)
{
  // Normalize inputs
  if (hour12 < 1)  hour12 = 1;
  if (hour12 > 12) hour12 = 12;
  if (minute < 0)  minute = 0;
  if (minute > 59) minute = 59;

  // Preserve current AM/PM from displayed time
  int currentHour24 = rtc.getHour();
  int ampm = (currentHour24 >= 12) ? 1 : 0;

  // Convert to local 24-hour
  int localHour24 = hour12 % 12;
  if (ampm == 1) localHour24 += 12;

  // Preserve date
  int year  = isValid() ? rtc.getYear()  : 2025;
  int month = isValid() ? rtc.getMonth() : 1;
  int day   = isValid() ? rtc.getDay()   : 1;
  int sec   = isValid() ? rtc.getSecond() : 0;

  // ---- KEY FIX ----
  // Convert local time → UTC before setting
  int utcHour24 = localHour24 - (rtc.offset / 3600);
  if (utcHour24 < 0)  utcHour24 += 24;
  if (utcHour24 > 23) utcHour24 -= 24;

  rtc.setTime(sec, minute, utcHour24, day, month, year);

  Serial.printf(
    "RTC setHourMinute (local %02d:%02d -> UTC %02d:%02d) epoch %lu offset %ld\n",
    localHour24, minute,
    utcHour24, minute,
    (unsigned long)rtc.getEpoch(),
    (long)rtc.offset
  );
}

// -----------------------------------------------------------------------------
// Pacific TZ helper (PST/PDT) using POSIX TZ rules:
// PST8PDT,M3.2.0/2,M11.1.0/2
//  - DST starts: 2nd Sunday in March at 02:00
//  - DST ends:   1st Sunday in November at 02:00
// -----------------------------------------------------------------------------
static bool pacificIsDST(time_t utcEpoch)
{
  // Ensure TZ rules are set (safe to call repeatedly)
  setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
  tzset();

  struct tm localTm;
  localtime_r(&utcEpoch, &localTm);
  return (localTm.tm_isdst > 0);
}

/*
  This will eventually change to include GPS support to determine the time zone and standard time
*/

bool RealTimeClock::syncWithNTP(const char* ntpServer, uint32_t timeoutMs)
{
  Serial.printf("Starting NTP sync with %s (timeout %lums)\n", ntpServer, timeoutMs);

  // 1) Get UTC time from NTP. Keep TZ logic separate.
  configTime(0, 0, ntpServer);

  uint32_t start = millis();
  time_t nowUtc = 0;

  while (millis() - start < timeoutMs)
  {
    time(&nowUtc);
    if (nowUtc > 1609459200) // sanity: > 2021-01-01
      break;

    delay(100);
  }

  if (nowUtc <= 1609459200)
  {
    Serial.println("NTP sync failed: timeout (no valid epoch)");
    return false;
  }

  // 2) Determine PST vs PDT for this date/time
  bool isDst = pacificIsDST(nowUtc);
  long pacificOffset = isDst ? -25200L : -28800L;   // PDT or PST

  // Keep your class state aligned with what ESP32Time uses
  _utcOffsetSeconds = (int)pacificOffset;
  rtc.offset = pacificOffset;

  // 3) Set RTC to the UTC epoch; ESP32Time will display with rtc.offset
  rtc.setTime(nowUtc);

  // Friendly debug print: show UTC and local
  struct tm localTm;
  localtime_r(&nowUtc, &localTm);

  Serial.printf("NTP acquired UTC epoch: %lu\n", (unsigned long)nowUtc);
  Serial.printf("Pacific: %s offset %ld\n", isDst ? "PDT" : "PST", pacificOffset);
  Serial.printf("Local: %04d-%02d-%02d %02d:%02d:%02d\n",
                localTm.tm_year + 1900, localTm.tm_mon + 1, localTm.tm_mday,
                localTm.tm_hour, localTm.tm_min, localTm.tm_sec);

  return true;
}

void RealTimeClock::loop()
{
}

