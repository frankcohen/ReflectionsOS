/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "RealTimeClock.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

RealTimeClock::RealTimeClock(){}

void RealTimeClock::begin()
{ 
  //Serial.println( "RealTimeClock begin");

  struct tm timeinfo;

  if ( ! getLocalTime( &timeinfo ) ) 
  {
    Serial.println( "RTC using default values 2, 50" );

    // Set default date and time (in UTC)

    struct tm timeinfo = {0};

    timeinfo.tm_year = 2024 - 1900;  // tm_year is years since 1900
    timeinfo.tm_mon = 4 - 1;     // tm_mon is 0-based (0 = January)
    timeinfo.tm_mday = 23;          // Day of the month
    timeinfo.tm_hour = 2;         // Hour
    timeinfo.tm_min = 50;        // Minute
    timeinfo.tm_sec = 0;        // Second

    time_t t = mktime(&timeinfo);       // Convert struct tm to time_t (seconds since the Unix epoch)
    struct timeval tv = {t, 0};  // time_t and microseconds
    settimeofday(&tv, nullptr);  // Set the time on the ESP32
  }
}

// Function to get the current hour from the RTC

int RealTimeClock::getHour() 
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) 
  {
    return timeinfo.tm_hour;  // Return the current hour from RTC
  } 
  else 
  {
    Serial.println("Failed to get local time from RTC.");
    return -1;  // Return -1 if time cannot be obtained
  }
}

// Function to get the current minute from the RTC

int RealTimeClock::getMinute() 
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) 
  {
    return timeinfo.tm_min;  // Return the current minute from RTC
  } 
  else 
  {
    Serial.println("Failed to get local time from RTC.");
    return -1;  // Return -1 if time cannot be obtained
  }
}

void RealTimeClock::loop()
{
}
