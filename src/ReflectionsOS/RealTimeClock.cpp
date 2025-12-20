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
  snprintf(buf, sizeof(buf), "%02d:%02d", rtc.getHour(), rtc.getMinute());
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

void RealTimeClock::setTimeStruct(const struct tm& t) {
  rtc.setTimeStruct(t);

  Serial.printf(
    "RTC setTimeStruct: %04d-%02d-%02d %02d:%02d:%02d (epoch %lu)\n",
    rtc.getYear(), rtc.getMonth(), rtc.getDay(),
    rtc.getHour(), rtc.getMinute(), rtc.getSecond(),
    (unsigned long)rtc.getEpoch()
  );
}

/*
  This will eventually change to include GPS support to determine the time zone 
  For now it forces Pacific Standard Time zone
*/

bool RealTimeClock::syncWithNTP(const char* ntpServer, uint32_t timeoutMs) {
  Serial.printf("Starting NTP sync with %s (timeout %lums)\n", ntpServer, timeoutMs);

  // Set the time zone offset to Pacific Standard Time (PST) using configTime
  configTime(-32400, 3600, ntpServer);  // PST (GMT-8) with 1-hour daylight saving time offset (3600)

  struct tm timeinfo;
  uint32_t start = millis();
  
  // Wait for NTP sync to complete within the specified timeout
  while (millis() - start < timeoutMs) {
    if (getLocalTime(&timeinfo)) {
      // Sync successful, set the time to the ESP32Time object
      rtc.setTimeStruct(timeinfo);
      time_t now = rtc.getEpoch();
      Serial.printf("NTP time acquired: %s", ctime(&now));
      rtc.setTime( now );  // Set RTC to the provided time
      return true;
    }
    delay(100);  // Try again every 100 ms
  }

  // NTP sync failed within timeout
  Serial.println("NTP sync failed: timeout");
  return false;
}

void RealTimeClock::loop()
{
}

