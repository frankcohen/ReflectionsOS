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
  _wifiMulti.addAP( SECRET_SSID, SECRET_PASS );

  //Serial.println( F( "Connecting Wifi" ) );

  long ago = millis();
  while ( millis() < ( ago + ( 1000*10 ) ) ) { break; }

  if ( _wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println( F( "WiFi not connected" ) );
  }
  else
  {
    Serial.println( F( "WiFi connected" ) );
  }
}

void Wifi::loop()
{}
