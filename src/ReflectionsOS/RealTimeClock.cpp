/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "RealTimeClock.h"
#include <Preferences.h>
#include <esp_sleep.h>

// Global RTC instance used by ESP32Time
ESP32Time rtc(0);

// NVS storage keys
static Preferences sPrefs;
static const char* kRtcNs   = "rtc";
static const char* kH12K    = "h12";     // uint8 1..12
static const char* kMinK    = "min";     // uint8 0..59
static const char* kAmPmK   = "ampm";    // uint8 0/1
static const char* kSecK    = "sec";     // uint8 0..59 (optional)
static const char* kMagicK  = "magic";   // uint32 marker

static const uint32_t kMagic = 0xC0C0A11FUL;

// How often to autosave to NVS
static const uint32_t kSaveEveryMs = 120000UL; // 2 minutes

RealTimeClock::RealTimeClock() {}

static bool isDeepSleepWake_()
{
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  // Anything other than UNDEFINED generally implies we woke from deep sleep.
  // (Your accel wake is usually EXT0/EXT1 depending on wiring.)
  return (cause != ESP_SLEEP_WAKEUP_UNDEFINED);
}

void RealTimeClock::begin(int utcOffsetSeconds)
{
  _utcOffsetSeconds = utcOffsetSeconds;

  // You don't care about date/timezone for the Cat's display.
  // Keep offset at 0 so rtc.getHour()/getMinute() match what you set.
  rtc.offset = 0;

  // If we woke from deep sleep, do NOT overwrite RTC with NVS.
  // This preserves your requirement that time continues through deep sleep.
  if (!isDeepSleepWake_()) {
    ensureClockValid_();
  } else {
    // Deep sleep wake: assume RTC kept counting.
    // Still ensure we don't ever show --:-- by forcing valid if NVS exists
    // and RTC looks nonsensical (rare).
    // In practice rtc.getHour() is always 0..23, so just mark valid.
    _hasValidClock = true;
  }

  Serial.printf(
    "RTC(begin): hour=%02d min=%02d sec=%02d epoch=%lu (deepWake=%d)\n",
    rtc.getHour(), rtc.getMinute(), rtc.getSecond(),
    (unsigned long)rtc.getEpoch(),
    isDeepSleepWake_() ? 1 : 0
  );

  if (_lastBuzzedHour < 0) {
    _lastBuzzedHour = rtc.getHour();
    return;
  }
}

int RealTimeClock::getHour()   { return rtc.getHour(); }
int RealTimeClock::getMinute() { return rtc.getMinute(); }
int RealTimeClock::getSecond() { return rtc.getSecond(); }

void RealTimeClock::getHour12AmPm_(int& outHour12, int& outAmPm) const
{
  int h24 = rtc.getHour(); // 0..23
  outAmPm = (h24 >= 12) ? 1 : 0;

  int h12 = h24 % 12;
  if (h12 == 0) h12 = 12;

  outHour12 = h12;
}

String RealTimeClock::getTime()
{
  if (!_hasValidClock) {
    ensureClockValid_();
  }

  int h12 = 12;
  int ap  = 0;
  getHour12AmPm_(h12, ap);

  char buf[6]; // "h:mm" up to "12:59"
  snprintf(buf, sizeof(buf), "%d:%02d", h12, rtc.getMinute());
  return String(buf);
}

void RealTimeClock::setClock12_(int hour12, int minute, int ampm, int second)
{
  // Normalize
  if (hour12 < 1)  hour12 = 1;
  if (hour12 > 12) hour12 = 12;
  if (minute < 0)  minute = 0;
  if (minute > 59) minute = 59;
  if (second < 0)  second = 0;
  if (second > 59) second = 59;
  ampm = (ampm != 0) ? 1 : 0;

  // Convert to 24-hour
  int hour24 = hour12 % 12;      // 12 -> 0
  if (ampm == 1) hour24 += 12;   // PM adds 12

  // Dummy date internally (never displayed)
  const int year  = 2025;
  const int month = 1;
  const int day   = 1;

  rtc.setTime(second, minute, hour24, day, month, year);
  _hasValidClock = true;
}

void RealTimeClock::setDefaultClock_()
{
  // Requirement: if no NVS clock, default to 12:00 (midnight)
  // midnight = 12:00 AM
  setClock12_(12, 0, 0, 0);
}

bool RealTimeClock::loadClockFromNVS()
{
  sPrefs.begin(kRtcNs, true);
  const uint32_t magic = sPrefs.getULong(kMagicK, 0);
  const uint8_t  h12   = sPrefs.getUChar(kH12K, 0);
  const uint8_t  m     = sPrefs.getUChar(kMinK, 255);
  const uint8_t  ap    = sPrefs.getUChar(kAmPmK, 2);
  const uint8_t  s     = sPrefs.getUChar(kSecK, 0);
  sPrefs.end();

  if (magic != kMagic) return false;
  if (h12 < 1 || h12 > 12) return false;
  if (m > 59) return false;
  if (ap > 1) return false;
  if (s > 59) return false;

  setClock12_(h12, m, ap, s);
  return true;
}

void RealTimeClock::saveClockToNVS()
{
  if (!_hasValidClock) return;

  int h12 = 12;
  int ap  = 0;
  getHour12AmPm_(h12, ap);

  const uint8_t m = (uint8_t)rtc.getMinute();
  const uint8_t s = (uint8_t)rtc.getSecond();

  sPrefs.begin(kRtcNs, false);
  sPrefs.putULong(kMagicK, kMagic);
  sPrefs.putUChar(kH12K, (uint8_t)h12);
  sPrefs.putUChar(kMinK, m);
  sPrefs.putUChar(kAmPmK, (uint8_t)ap);
  sPrefs.putUChar(kSecK, s);
  sPrefs.end();
}

void RealTimeClock::ensureClockValid_()
{
  // If RTC is already running and we've previously validated, do nothing.
  if (_hasValidClock) return;

  // Try NVS first
  if (loadClockFromNVS()) {
    _hasValidClock = true;
    return;
  }

  // No NVS clock: default to 12:00 and persist it
  setDefaultClock_();
  saveClockToNVS();
  _hasValidClock = true;
}

void RealTimeClock::setTime(int hour12, int minute, int ampm)
{
  setClock12_(hour12, minute, ampm, 0);

  Serial.printf("RTC setTime: %s (%02d:%02d:%02d 24h)\n",
                getTime().c_str(),
                rtc.getHour(), rtc.getMinute(), rtc.getSecond());

  saveClockToNVS();
}

void RealTimeClock::setHourMinute(int hour12, int minute)
{
  // Preserve current AM/PM
  int currentH12 = 12;
  int currentAp  = 0;
  getHour12AmPm_(currentH12, currentAp);

  setClock12_(hour12, minute, currentAp, rtc.getSecond());

  Serial.printf("RTC setHourMinute: %s (%02d:%02d:%02d 24h)\n",
                getTime().c_str(),
                rtc.getHour(), rtc.getMinute(), rtc.getSecond());

  saveClockToNVS();
}

void RealTimeClock::setEpoch(time_t epoch)
{
  // If you ever set epoch (NTP, etc.), accept it.
  // We still only *persist* the clock components.
  rtc.setTime(epoch);
  _hasValidClock = true;
  saveClockToNVS();

  Serial.printf("RTC setEpoch: epoch=%lu time=%s\n",
                (unsigned long)rtc.getEpoch(),
                getTime().c_str());
}

time_t RealTimeClock::getEpoch()
{
  return (time_t)rtc.getEpoch();
}

bool RealTimeClock::syncWithNTP(const char* ntpServer, uint32_t timeoutMs)
{
  Serial.printf("Starting NTP sync with %s (timeout %lums)\n", ntpServer, timeoutMs);

  // Request UTC epoch from NTP
  configTime(0, 0, ntpServer);

  uint32_t start = millis();
  time_t nowUtc = 0;

  while (millis() - start < timeoutMs)
  {
    time(&nowUtc);
    if (nowUtc > 1609459200) // > 2021-01-01 sanity
      break;

    delay(100);
  }

  if (nowUtc <= 1609459200)
  {
    Serial.println("NTP sync failed: timeout (no valid epoch)");
    return false;
  }

  // Set RTC to the acquired epoch.
  // Cat doesn't care about date; we just use it as a time base.
  rtc.setTime(nowUtc);
  _hasValidClock = true;

  Serial.printf("NTP acquired epoch=%lu time=%s\n",
                (unsigned long)nowUtc,
                getTime().c_str());

  // Persist the clock components
  saveClockToNVS();

  return true;
}

void RealTimeClock::loop()
{
  // Haptic buzz on hour change
  const int hourNow = rtc.getHour();
  if (_lastBuzzedHour < 0) _lastBuzzedHour = hourNow;

  if (hourNow != _lastBuzzedHour)
  {
    haptic.playEffect(72);
    _lastBuzzedHour = hourNow;
  }

  // Requirement: save every 2 minutes
  static uint32_t lastSaveMs = 0;
  const uint32_t nowMs = millis();
  if (nowMs - lastSaveMs >= kSaveEveryMs) {
    lastSaveMs = nowMs;
    saveClockToNVS();
  }
}