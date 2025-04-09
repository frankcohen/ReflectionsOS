/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _LED_
#define _LED_

#include "config.h"
#include "secrets.h"

// #define FASTLED_INTERNAL
// #include <FastLED.h>

#include "Arduino.h"

class LED
{
  public:
    LED();
    void begin();
    void loop();

    void show();
    void setBlue( int lednum );
    void setAllBlack();

  private:
    //CRGB leds[ LED_Count ];
};

#endif // _LED_
