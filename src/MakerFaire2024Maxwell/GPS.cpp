/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Uses of a TinyGPSPlus (TinyGPSPlus) object to decode GPS module data
 See config.h for pin definitions
*/

#include "GPS.h"

GPS::GPS(){}

void GPS::begin()
{ 
  Serial2.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  active = false;
  long gpstime = millis();
  
  //configTime(0, 0, "pool.ntp.org"); // Use NTP server for initial sync (optional)
}

bool GPS::isActive()
{
  return active;
}

bool GPS::test()
{
  // waits up to 5 seconds to get data from the GPS module

  long time = millis();
  while ( millis() < time + 5000 )
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
    
    if ( gps.charsProcessed() > 0 )
    {
      return true;
    }
  }
  return false;
}

void GPS::printHeader()
{
  Serial.print(F("Testing TinyGPSPlus library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
}

void GPS::printLocation()
{
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;

  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);

  unsigned long distanceKmToLondon =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON) / 1000;
  printInt(distanceKmToLondon, gps.location.isValid(), 9);

  double courseToLondon =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON);

  printFloat(courseToLondon, gps.location.isValid(), 7, 2);

  const char *cardinalToLondon = TinyGPSPlus::cardinal(courseToLondon);

  printStr(gps.location.isValid() ? cardinalToLondon : "*** ", 6);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();
}

void GPS::printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
}

void GPS::printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
}

void GPS::printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
}

void GPS::printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
}

void GPS::loop()
{
  while ( Serial2.available() > 0 ) 
  {
    gps.encode( Serial2.read() );
  }

  if ( ( millis() - gpstime ) > 1000 * 60 * 60 * 3 )
  {
    gpstime = millis();

    if ( gps.charsProcessed() > 0 ) 
    {
      // Check if a valid location has been obtained
      if ( gps.location.isValid() && gps.date.isValid() && gps.time.isValid() )
      {
        active = true;

        // Get the time from the GPS
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
        int day = gps.date.day();
        int month = gps.date.month();
        int year = gps.date.year();

        // Print the GPS time for debugging
        Serial.printf("GPS Time: %02d:%02d:%02d %02d/%02d/%04d\n", hour, minute, second, day, month, year);

        // Set the ESP32 internal RTC
        struct tm timeinfo;
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;
        time_t t = mktime(&timeinfo);
        struct timeval now = { .tv_sec = t };
        settimeofday(&now, NULL);

        // Print the RTC time for debugging
        time_t now_time = time(NULL);
        struct tm * timeinfo_now = localtime(&now_time);
        Serial.printf("RTC Time: %02d:%02d:%02d %02d/%02d/%04d\n", timeinfo_now->tm_hour, timeinfo_now->tm_min, timeinfo_now->tm_sec, timeinfo_now->tm_mday, timeinfo_now->tm_mon + 1, timeinfo_now->tm_year + 1900);

        // Print number of satellites
        Serial.printf("Satellites: %d\n", gps.satellites.value());
      }
    }
  }
}
