/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef WATCHFACEMAIN_H
#define WATCHFACEMAIN_H

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Video.h"
#include "TextMessageService.h"
#include "RealTimeClock.h"

#include <PNGdec.h>
#include <Arduino_GFX_Library.h>

#include "WatchFaceBase.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Battery battery;
extern RealTimeClock realtimeclock;

extern Arduino_GFX *gfx;
extern Arduino_Canvas *bufferCanvas;

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    

    enum Panel { MAIN, TIME, TIMER, HEALTH };
    
  private:
    void updateDisplay();

    void updateBlink();
    void updateHoursAndMinutes();
    void updateBattery();
    bool startBlinkAnimation();

    int panel;

    unsigned long maintimer;
    
    bool displayUpdateable;

    int currentHour, currentHour2, currentMinute, currentMinute2;
    int oldHour, oldMinute, oldBattery, oldBlink;

    unsigned long battimer;
    int batcount;
    int batlev;

    bool blinking;
    unsigned long catFaceTimer;
    int catFaceWait;
    int catFaceIndex;
    bool catFaceDirection;

    bool drawitall;
};

#endif // WATCHFACE_H
