/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _HAPTIC_
#define _HAPTIC_

#include "config.h"
#include "secrets.h"

#include "Adafruit_DRV2605.h"

class Haptic
{
  public:
    Haptic();
    void begin();
    bool test();
    void loop();
    void playEffect( int effect );

  private:
    Adafruit_DRV2605 drv;
    bool started;
};

#endif // _HAPTIC_
