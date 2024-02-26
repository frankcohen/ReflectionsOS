/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the Wifi network connection

Depends on WiFiManager by Tzapu, https://github.com/tzapu/WiFiManager

*/

#ifndef _Wifi_
#define _Wifi_

#include "config.h"
#include <WiFiUdp.h>
#include "secrets.h"
#include <WiFiMulti.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

class Wifi
{
  public:
    Wifi();
    void begin();
    void reset();
    void loop();
    bool isConnected();

  private:
    String devicename;
    long lastWifiTime;
};

#endif // _Wifi_
