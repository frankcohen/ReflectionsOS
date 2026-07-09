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

static const __FlashStringHelper *wifiStatusName( wl_status_t status )
{
  switch ( status )
  {
    case WL_IDLE_STATUS: return F("WL_IDLE_STATUS");
    case WL_NO_SSID_AVAIL: return F("WL_NO_SSID_AVAIL");
    case WL_SCAN_COMPLETED: return F("WL_SCAN_COMPLETED");
    case WL_CONNECTED: return F("WL_CONNECTED");
    case WL_CONNECT_FAILED: return F("WL_CONNECT_FAILED");
    case WL_CONNECTION_LOST: return F("WL_CONNECTION_LOST");
    case WL_DISCONNECTED: return F("WL_DISCONNECTED");
    default: return F("WL_UNKNOWN_STATUS");
  }
}

bool Wifi::begin()
{ 
  Serial.println(F("Wifi begin"));
  Serial.print(F("Connecting to SSID: "));
  Serial.println(SECRET_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  delay(100);
  WiFi.begin( SECRET_SSID, SECRET_PASS);

  const unsigned long wifiTimeoutMs = 20000;
  const unsigned long statusPrintMs = 1000;
  unsigned long started = millis();
  unsigned long lastStatusPrint = 0;

  // Wait for connection, but never hang forever.
  while ( WiFi.status() != WL_CONNECTED )
  {
    unsigned long now = millis();

    if ( ( now - lastStatusPrint ) >= statusPrintMs )
    {
      wl_status_t status = WiFi.status();
      Serial.print(F("WiFi waiting, status="));
      Serial.print((int) status);
      Serial.print(F(" "));
      Serial.println(wifiStatusName(status));
      lastStatusPrint = now;
    }

    if ( ( now - started ) >= wifiTimeoutMs )
    {
      wl_status_t status = WiFi.status();
      Serial.print(F("WiFi connection timed out after ms="));
      Serial.println(wifiTimeoutMs);
      Serial.print(F("Final WiFi status="));
      Serial.print((int) status);
      Serial.print(F(" "));
      Serial.println(wifiStatusName(status));
      Serial.println(F("Check SECRET_SSID/SECRET_PASS in secrets.h and confirm the network is 2.4 GHz."));
      WiFi.disconnect(false, false);
      return false;
    }

    delay(100);
  }

  Serial.println(F("WiFi connected"));

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
