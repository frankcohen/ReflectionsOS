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

#define transparent_color 0xFFE0

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    
    
  private:
    void updateDisplay();

    // Hours and minutes animate clockwise and counterclockwist into position
    bool startupAnimation();
    void beginStartupAnimation();

    void blinks();
    void timelyHoursAndMinutes();
    void batteryMove();

    uint32_t lastUpdate;

    bool displayUpdateable;

    int counter;
    int startHour;
    int startMinute;
    int currentHour, currentMinute;
    bool startupComplete;
    unsigned long hoursmintimer;
    bool honce;
    bool monce;

    unsigned long battimer;
    int batcount;
    int batlev;

    unsigned long catFaceTimer;
    int catFaceWait;
    bool blinking;
    int catFaceIndex;
    bool catFaceDirection;

    long facetime;
};

#endif // WATCHFACE_H
