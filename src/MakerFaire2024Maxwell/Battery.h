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
#include "Video.h"

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts

#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

#define batterylow 3900
#define battermedium 4000
#define batteryhigh 4000

extern Arduino_GFX *gfx;

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
    int16_t x, y;
    uint16_t w, h;
    
    void drawSpriteOverBackground( const uint16_t *sprite, int16_t spriteWidth, int16_t spriteHeight, int16_t x, int16_t y, uint16_t transparent );
};

#endif // _BATTERY_
