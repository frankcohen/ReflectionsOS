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

ESP32Time rtc(0); 

RealTimeClock::RealTimeClock() {}

void RealTimeClock::begin() {
  rtc.offset = -32400;  // // offset in seconds GMT+1, this is for Pacific Standard Time (PST)

  Serial.printf(
    "RTC: %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
    rtc.getYear(), rtc.getMonth(), rtc.getDay(),
    rtc.getHour(), rtc.getMinute(),
    (unsigned long) rtc.getEpoch()
  );

  _prefs.begin(NVS_NAMESPACE, true);
  unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
  _prefs.end();
  if (before) 
  {
    // Print the NVS saved time
    Serial.printf(
      "RTC NVS = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      rtc.getYear(), rtc.getMonth(), rtc.getDay(),
      rtc.getHour(), rtc.getMinute(), before
    );
  } 

  // if we woke from a deep-sleep
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  if ( cause == ESP_SLEEP_WAKEUP_EXT1) 
  {
    _lastSaveMs = millis();
    _state = State::Done;
    return;
  }

  // otherwise, proceed with normal sync
  _state      = State::CheckRTC;
  _lastSaveMs = millis();
}

// Returns the nearest whole-hour offset east of UTC.
// e.g. lon=+30° → +2h, lon=–75° → –5h
static int computeOffsetFromLongitude(float lon) {
  return int(std::round(lon / 15.0f));
}

String RealTimeClock::lookupTimeZone(double lat, double lon) 
{
 int offset = int(round(lon / 15.0));
  char buf[16];
  if (offset >= 0) snprintf(buf, sizeof(buf), "GMT+%d", offset);
  else           snprintf(buf, sizeof(buf), "GMT%d", offset);
  return String(buf);
}
 
void RealTimeClock::applyTimestamp(time_t t, bool saveToNVS) 
{
  rtc.setTime(t);  // Set RTC to the provided epoch time

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

  if ( nowMs - _lastSaveMs < NVS_SAVE_INTERVAL_MS ) return;
  _lastSaveMs = nowMs;

  time_t now = rtc.getEpoch();  // Get the current epoch time from RTC

  // Check if the current year is valid, if not, set to Jan 1 2025
  int currentYear = rtc.getYear();
  if (currentYear < MIN_VALID_YEAR) 
  {
    Serial.println("PeriodicSave: RTC invalid, fixing date to Jan 1 2025");

    // Manually set the epoch time to Jan 1, 2025, 00:00 (midnight UTC)
    const int SECONDS_IN_A_DAY = 86400; // Number of seconds in a day
    const int SECONDS_IN_A_YEAR = SECONDS_IN_A_DAY * 365; // Rough estimate for 365 days
    time_t jan1_2025_epoch = 55 * SECONDS_IN_A_YEAR;  // Rough epoch time for 55 years after 1970
    
    // Set the corrected time in the RTC (no time zone offset adjustment needed)
    rtc.setTime( jan1_2025_epoch );
    now = rtc.getEpoch();  // Update the "now" with the fixed epoch time
  }

  // Print the current RTC time
  Serial.printf(
    "PeriodicSave: RTC now = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
    rtc.getYear(), rtc.getMonth(), rtc.getDay(),
    rtc.getHour(), rtc.getMinute(),
    (unsigned long) rtc.getEpoch()
  );

  // Read & show the old NVS value
  _prefs.begin(NVS_NAMESPACE, true);
  unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
  _prefs.end();
  if (before) 
  {
    // Print the NVS saved time
    Serial.printf(
      "PeriodicSave: NVS before = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      rtc.getYear(), rtc.getMonth(), rtc.getDay(),
      rtc.getHour(), rtc.getMinute(), before
    );
  } 
  else 
  {
    Serial.println("PeriodicSave: NVS before = <none>");
  }

  // Write the corrected epoch into NVS
  _prefs.begin(NVS_NAMESPACE, false);
  _prefs.putULong(NVS_KEY_LAST_TS, (unsigned long) rtc.getLocalEpoch() );
  _prefs.end();

  // Read & show the new NVS value (properly converted)
  _prefs.begin(NVS_NAMESPACE, true);
  unsigned long after = _prefs.getULong(NVS_KEY_LAST_TS, 0);
  _prefs.end();

  if (after) {
    // Set RTC to the new time after saving (epoch time should match)
    rtc.setTime(after);

    // Print the NVS updated time (should not apply any time zone adjustments)
    Serial.printf(
      "PeriodicSave: NVS after  = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
      rtc.getYear(), rtc.getMonth(), rtc.getDay(),
      rtc.getHour(), rtc.getMinute(), after
    );
  }
}

// Get the current hour from ESP32Time
int RealTimeClock::getHour() 
{
  return rtc.getHour();  // Directly fetch the hour from ESP32Time
}

// Get the current minute from ESP32Time
int RealTimeClock::getMinute() 
{
  return rtc.getMinute();  // Directly fetch the minute from ESP32Time
}

// Set the time based on the given hour, minute, and AM/PM
void RealTimeClock::setTime(int hour, int minute, int ampm) 
{
  rtc.setTime( 0, minute, ( hour - 3 ) % 12, 23, 4, 2025);

  // Print the set time to verify
  Serial.printf("Time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                rtc.getYear(), rtc.getMonth(), rtc.getDay(),
                rtc.getHour(), rtc.getMinute(), rtc.getSecond());
}

String RealTimeClock::getTime() 
{
  // Get the current time from the RTC
  int currentHour = rtc.getHour();
  int currentMinute = rtc.getMinute();

  // Validate RTC time (check if hour is between 0 and 23, and minute is between 0 and 59)
  if ((currentHour < 0 || currentHour > 23) || (currentMinute < 0 || currentMinute > 59)) {
    // RTC time is invalid, fall back to NVS value
    time_t saved;
    if (loadFromNVS(saved)) {
      // Load the saved time from NVS
      rtc.setTime(saved);  // Set RTC to the saved time
      currentHour = rtc.getHour();
      currentMinute = rtc.getMinute();
      Serial.printf("Loaded from NVS: %02d:%02d\n", currentHour, currentMinute);
    } else {
      // NVS value is also invalid, set the time to default (10:10 AM)
      currentHour = 10;
      currentMinute = 10;
      rtc.setTime(currentHour * 3600 + currentMinute * 60);  // Set default time to 10:10 AM
      Serial.println("No valid time in RTC or NVS, set to 10:10 AM");
    }
  }

  // Format the time as "HH:MM"
  char buf[6];
  snprintf(buf, sizeof(buf), "%d:%02d", currentHour, currentMinute);
  return String(buf);
}

// Static helper to check for a "reasonable" time
static bool timeIsValid(time_t t) {
  // Create an ESP32Time object
  ESP32Time rtc2;

  // Set the time in RTC
  rtc2.setTime(t);

  // Get the year using the ESP32Time API
  int currentYear = rtc2.getYear();

  // Check if the year is greater than or equal to MIN_VALID_YEAR
  if (currentYear < MIN_VALID_YEAR) {
    return false;
  }

  // Optionally, you can check for a future year limit
  if (currentYear > 2050) {
    return false;
  }

  return true;
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
      applyTimestamp(now, true);  // Apply the time to the RTC and save to NVS
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
  periodicSave();  // periodic NVS save every 1 minute

  switch (_state) 
  {

    case State::CheckRTC: 
    {
      if (rtc.getYear() > MIN_VALID_YEAR) 
      {
        // RTC is valid, print time
        Serial.printf("RTC valid: %02d:%02d %02d\n", rtc.getHour(), rtc.getMinute(), rtc.getYear());
        _state = State::Done;
      } 
      else 
      {
        // RTC is invalid, try syncing with GPS
        Serial.print("RTC invalid, " );
        Serial.print( rtc.getYear() );
        Serial.println( " trying GPS sync");
        gps.on();
        _gpsStartMs = millis();
        _state = State::GPSWait;
      }
      break;
    }

    case State::GPSWait:
      gps.loop();

      // If we have sufficient GPS data or timeout has occurred, proceed
      if (gps.getProcessed() >= GPS_SENTENCE_THRESHOLD || (millis() - _gpsStartMs) >= GPS_TIMEOUT_MS) 
      {
        // If GPS has a valid time and sufficient satellites, set RTC
        if ((gps.getHour() != 0 || gps.getMinute() != 0) && gps.getSatellites() > 2) 
        {
          // Compute offset from GPS longitude
          float lon = gps.getLng();
          int offs = computeOffsetFromLongitude(lon);
          Serial.printf("Longitude %.3f → offset %+d\n", lon, offs);

          // Note: Doing nothing with GPS offset at the moment, during debugging, todo later
          rtc.setTime( 0, gps.getMinute(), ( gps.getHour() - 3 ) % 12, gps.getDay(), gps.getMonth(), gps.getYear());

          Serial.printf("GPS local time: %d-%d-%d %02d:%02d\n", gps.getYear(), gps.getMonth(), gps.getDay(), gps.getHour(), gps.getMinute() );
          _state = State::Done;
        }
      }

      else
      {
        // Read & show the old NVS value
        _prefs.begin(NVS_NAMESPACE, true);
        unsigned long before = _prefs.getULong(NVS_KEY_LAST_TS, 0);
        _prefs.end();
        if (before) 
        {
          time_t epochTime = (time_t)before;  // Cast to time_t
          struct tm timeinfo;
          localtime_r(&epochTime, &timeinfo);  // Localtime function to get time components

          // Extract year, month, day, hour, and minute from struct tm
          int year = timeinfo.tm_year + 1900;  // tm_year is years since 1900
          int month = timeinfo.tm_mon + 1;     // tm_mon is 0-based, so we add 1
          int day = timeinfo.tm_mday;          // Day of the month
          int hour = timeinfo.tm_hour;         // Hour of the day (0-23)
          int minute = timeinfo.tm_min;        // Minute of the hour (0-59)

          // Print the derived date and time
          Serial.printf(
            "NVS = %04d-%02d-%02d %02d:%02d (epoch %lu)\n",
            year, month, day, hour, minute, before
          );

          rtc.setTime( 0, minute, hour, day, month, year);
          Serial.printf("RTC is now: %02d:%02d %02d\n", rtc.getHour(), rtc.getMinute(), rtc.getYear());
          _state = State::Done;
        }
        else
        {
          rtc.setTime( 0, 10, 10, 23, 4, 2025);          
          _state = State::Done;
        }
      }              
      break;

    case State::ApplyTime:
    {
      // Print the new RTC time
      int currentYear = rtc.getYear();
      int currentHour = rtc.getHour();
      int currentMinute = rtc.getMinute();

      Serial.printf("RTC now %02d:%02d %04d\n", currentHour, currentMinute, currentYear);
      _state = State::Done;
      break;
    }

    case State::Done:
      break;
  }
}
