/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the video display.

*/

#include "Wifi.h"

Wifi::Wifi() {}

void Wifi::begin()
{ 
  //Serial.println("Wifi begin");

  WiFi.begin( SECRET_SSID, SECRET_PASS);

  std::string devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  devicename = devname.c_str();

  bool again = true;

  // Wait for connection
  while ( again )
  {
    int Wifistatus = WiFi.status();
    if ( Wifistatus == WL_CONNECTED) again = false;
    delay(1000);
  }

  // Once connected, print the IP address
  Serial.print("Connected to WiFi, ");
  Serial.println(WiFi.localIP());

  setRTCfromNTP();

  lastWifiTime = millis();
  checkNTP = millis();
}

/*
  WifiManager optionally auto-reconnects the device to the previous network
  connection. It stores the SSID and password in the device SPIFFS. reset()
  wipes stored credentials.
*/

void Wifi::reset()
{
}

/*
* Get the time from Net Time Protocol (NTP) service pool.ntp.org
* Set the internal RTC
*/

void Wifi::setRTCfromNTP()
{
  if ( WiFi.status() == WL_CONNECTED ) 
  {
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");

    struct tm timeinfo;
    if ( ! getLocalTime( &timeinfo ) ) 
    {
      Serial.println( "Net time not set" );
      return;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    String period = "AM";

    if (hour >= 12) {
      period = "PM";
      if (hour > 12) {
        hour -= 12;
      }
    } else if (hour == 0) {
      hour = 12; // Midnight case
    }

    String minuteStr = (minute < 10) ? "0" + String(minute) : String(minute);
    String timeStr = String(hour) + ":" + minuteStr + " " + period;

    Serial.print( "Net time set to " );
    Serial.print( timeStr );
    Serial.print( ", GMT offset " );
    Serial.println( gmtOffset_sec );
  }
  else
  {
    Serial.println( "Net time not set, no WIFI connection" );
  }
}

bool Wifi::isConnected()
{
  if ( WiFi.status() == 3 ) return true;
  return false;
}

bool Wifi::isTurnedOn()
{
  if ( WiFi.status() == WL_CONNECTED )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Wifi::loop()
{
  if ( (millis() - lastWifiTime) > 500)
  {
    lastWifiTime = millis();
  }

  if ( ( (millis() - checkNTP ) > ( 1 * (60 * 1000) ) ) && isConnected() )
  {
    checkNTP = millis();
    
    setRTCfromNTP();
  }

}
