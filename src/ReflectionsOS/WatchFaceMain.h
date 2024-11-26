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
extern AccelSensor accel;
extern Video video;
extern Arduino_GFX *gfx;
extern Arduino_Canvas *bufferCanvas;

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    

    enum Panel { 
      STARTUP, MAIN, 
      DISPLAYING_DIGITAL_TIME, SETTING_DIGITAL_TIME, 
      DISPLAYING_TIMER, SETTING_TIMER, 
      DISPLAYING_HEALTH_STATISTICS, SETTING_HEALTH_STATISTICS };
    
  private:
    void updateDisplayMain();

    void updateBlink();
    void updateHoursAndMinutes();
    void updateBattery();
    bool updateTimeLeft();

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

    int rotating;

    bool drawitall;
    bool needssetup;

    unsigned long noMovementTime;
    unsigned long myTeeTime;
};

#endif // WATCHFACE_H
