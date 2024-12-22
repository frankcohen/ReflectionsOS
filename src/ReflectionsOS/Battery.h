/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _BATTERY_
#define _BATTERY_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

#define batterylow 3900
#define battermedium 4000
#define batteryhigh 4000

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

class Battery
{
  public:
    Battery();
    void begin();
    void loop();
    bool test();
    String batLevel( float analogVolts );
    bool isBatteryLow();
    int getBatteryLevel();

  private:
    long batteryWaitTime;
    float analogVolts;
    
};

#endif // _BATTERY_
