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

static   WiFiManager wm;

void Wifi::begin()
{ 
  Serial.println("Wifi begin");
  
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  
  lastWifiTime = millis();

  // Automatically connects using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  std::string devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  devicename = devname.c_str();

  bool res = wm.autoConnect( devicename.c_str(), CALLIOPE_WIFI_PASS ); // password protected ap

  if(!res) {
    Serial.print( devicename );
    Serial.println(": WifiManager connect failed");
    // ESP.restart();
  } 
  else {
    Serial.print( devicename );
    Serial.println(": WifiManager connected");
  }
}

/*
  WifiManager optionally auto-reconnects the device to the previous network
  connection. It stores the SSID and password in the device SPIFFS. reset()
  wipes stored credentials.
*/

void Wifi::reset()
{
  wm.resetSettings();
}

bool Wifi::isConnected()
{
  if ( WiFi.status() == 3 ) return true;
  return false;
}

void Wifi::loop()
{
  if ( (millis() - lastWifiTime) > 2000)
  {
    lastWifiTime = millis();
    wm.process();     // Non-blocking Wifi manager
  }
}
