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

bool Wifi::begin()
{ 
  //Serial.println("Wifi begin");

  WiFi.begin( SECRET_SSID, SECRET_PASS);

  bool again = true;

  // Wait for connection
  while ( again )
  {
    int Wifistatus = WiFi.status();
    if ( Wifistatus == WL_CONNECTED) again = false;
    delay(1000);
  }

  if ( again ) return false;

  // Once connected, print the IP address
  Serial.print(F("Connected to WiFi, "));
  Serial.println(WiFi.localIP());

  setRTCfromNTP();

  lastWifiTime = millis();
  checkNTP = millis();

  return true;
}

// Looking for getDeviceName() ? See Wifi.h

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
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org" );

    struct tm timeinfo;
    if ( ! getLocalTime( &timeinfo ) ) 
    {
      Serial.println( F("Net time not set") );
      return;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    String period = F("AM");

    if (hour >= 12) {
      period = F("PM");
      if (hour > 12) {
        hour -= 12;
      }
    } else if (hour == 0) {
      hour = 12; // Midnight case
    }

    String minuteStr = (minute < 10) ? "0" + String(minute) : String(minute);
    String timeStr = String(hour) + ":" + minuteStr + " " + period;

    Serial.print( F("Net time set to ") );
    Serial.print( timeStr );
    Serial.print( F(", GMT offset ") );
    Serial.println( gmtOffset_sec );
  }
  else
  {
    Serial.println( F("Net time not set, no WIFI connection") );
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

String Wifi::getMACAddress()
{
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);

  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return macStr;
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
