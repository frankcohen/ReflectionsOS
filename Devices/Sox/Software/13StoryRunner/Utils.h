/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for general purpose tasks

*/

#ifndef _UTILS_
#define _UTILS_

#include <ArduinoJson.h>
#include "config.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

class Utils
{
  public:
    Utils();
    void begin();
    void smartdelay(unsigned long ms);

  private:

};

#endif // _UTILS_
