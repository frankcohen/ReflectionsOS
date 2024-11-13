/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef WATCHFACEEXPERIENCES_H
#define WATCHFACEEXPERIENCES_H

#include "config.h"
#include "secrets.h"

#include "WatchFaceMain.h"
#include <Arduino_GFX_Library.h>

enum WatchFaceState {
    MAIN,
    DISPLAYING_DIGITAL_TIME,
    SETTING_DIGITAL_TIME,
    DISPLAYING_HEALTH_STATISTICS,
    SETTING_HEALTH_STATISTICS,
    DISPLAYING_TIMER,
    SETTING_TIMER,
    DISPLAY_OFF
};

class WatchFaceExperiences 
{
  public:
    WatchFaceExperiences();
    void begin();
    void loop();
    void setState(WatchFaceState newState);
  private:
    WatchFaceState currentState;
    WatchFaceMain watchFaceMain;
};

#endif // WATCHFACEEXPERIENCES_H

