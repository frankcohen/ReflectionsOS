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
    bool isTurnedOn();
    void setRTCfromNTP();

  private:
    String devicename;
    long lastWifiTime;
    long checkNTP;

    // Timezone offset in seconds (e.g., for GMT+1, offset is 3600)
    const long  gmtOffset_sec = 3600 * -8;   // PST = GMT-8

    // Daylight saving time offset in seconds
    const int   daylightOffset_sec = 3600;
};

#endif // _Wifi_
