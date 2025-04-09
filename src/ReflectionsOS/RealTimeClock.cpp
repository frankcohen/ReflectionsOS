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
  //Serial.println( F("RealTimeClock begin"));

  struct tm timeinfo;

  if ( ! getLocalTime( &timeinfo ) ) 
  {
    struct tm timeinfo = {0};

    timeinfo.tm_year = 2024 - 1900;  // tm_year is years since 1900
    timeinfo.tm_mon = 4 - 1;     // tm_mon is 0-based (0 = January)
    timeinfo.tm_mday = 23;          // Day of the month
    timeinfo.tm_hour = 2;         // Hour
    timeinfo.tm_min = 50;        // Minute
    timeinfo.tm_sec = 0;        // Second

    bool setsup = true;
    gps.on();
    unsigned long gpsstartup = millis();

    while ( ( gps.getProcessed() < 1000 ) && ( millis() - gpsstartup < 5000 ) )
    {
      gps.loop();
    }

    /*
    Serial.print( F("GPS ") );
    Serial.print( millis() - gpsstartup );
    Serial.print( F(" ") );
    Serial.print( gps.getHour() );
    Serial.print( F(":") );
    Serial.print( gps.getMinute() );
    Serial.print( F(" ") );
    Serial.print( gps.getMonth() );
    Serial.print( F("/") );
    Serial.println( gps.getDay() );
    */

    if ( ! ( ( gps.getHour() == 0 ) && ( gps.getMinute() == 0 ) ) )
    {
      setsup = false;

      unsigned int hour = gps.getHour() + ( timeRegionOffset ) ;    // Set in config.h TODO this must be fixed
      unsigned int minute = gps.getMinute();

      Serial.print( F("RTC using GPS values ") );
      Serial.print( hour );
      Serial.print( F(":") );
      String mymin = F("");
      if ( minute < 10 ) mymin += F("0"); 
      mymin += String( minute );
      Serial.print( mymin );

      if ( hour > 12 ) hour = hour - 12;

      timeinfo.tm_hour = hour;         // Hour
      timeinfo.tm_min = minute;        // Minute
      timeinfo.tm_sec = 0;        // Second

      unsigned int month = gps.getMonth();
      unsigned int day = gps.getDay();
      unsigned int year = gps.getYear();

      Serial.print( F(", (mm/dd/yyyy) ") );
      Serial.print( month );
      Serial.print( F("/") );
      Serial.print( day );
      Serial.print( F("/") );
      Serial.println( year );

      timeinfo.tm_year = year - 1900;  // tm_year is years since 1900
      timeinfo.tm_mon = month - 1;     // tm_mon is 0-based (0 = January)
      timeinfo.tm_mday = day;          // Day of the month
    }

    gps.off();

    if ( setsup )
    {
      Serial.println( F("RTC using default values 2 hours, 50 minutes") );

      timeinfo.tm_hour = 2;         // Hour
      timeinfo.tm_min = 50;        // Minute
      timeinfo.tm_sec = 0;        // Second
    }

    time_t t = mktime(&timeinfo);       // Convert struct tm to time_t (seconds since the Unix epoch)
    struct timeval tv = {t, 0};  // time_t and microseconds
    settimeofday(&tv, nullptr);  // Set the time on the ESP32
  }
  else
  {
    Serial.print( F("Real time clock: ") );
    Serial.print( getHour() );
    Serial.print( F(":") );
    String mef = F("");
    if ( getMinute() < 10 ) mef += F("0");
    mef += getMinute();
    Serial.println( mef );
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
    Serial.println(F("Failed to get local time from RTC."));
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
    Serial.println(F("Failed to get local time from RTC."));
    return -1;  // Return -1 if time cannot be obtained
  }
}

void RealTimeClock::setTime( int hour, int minute, int ampm )
{
      // Set default date and time (in UTC)

    struct tm timeinfo = {0};
    if ( getLocalTime(&timeinfo) )
    {
      timeinfo.tm_year = 2024 - 1900;  // tm_year is years since 1900
      timeinfo.tm_mon = 4 - 1;     // tm_mon is 0-based (0 = January)
      timeinfo.tm_mday = 23;          // Day of the month
      timeinfo.tm_hour = hour;         // Hour
      timeinfo.tm_min = minute;        // Minute
      timeinfo.tm_sec = 0;        // Second
    }
    else
    {
      timeinfo.tm_year = 2024 - 1900;  // tm_year is years since 1900
      timeinfo.tm_mon = 4 - 1;     // tm_mon is 0-based (0 = January)
      timeinfo.tm_mday = 23;          // Day of the month
      timeinfo.tm_hour = 10;         // Hour
      timeinfo.tm_min = 10;        // Minute
      timeinfo.tm_sec = 0;        // Second
    }

    time_t t = mktime(&timeinfo);       // Convert struct tm to time_t (seconds since the Unix epoch)
    struct timeval tv = {t, 0};  // time_t and microseconds
    settimeofday(&tv, nullptr);  // Set the time on the ESP32
}

void RealTimeClock::loop()
{
}
