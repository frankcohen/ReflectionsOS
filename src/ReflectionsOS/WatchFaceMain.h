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
#include "AccelSensor.h"
#include "ExperienceService.h"
#include "Steps.h"
#include "Timer.h"
#include <Kalman.h>

#include <PNGdec.h>
#include <Arduino_GFX_Library.h>

#include "WatchFaceBase.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Battery battery;
extern RealTimeClock realtimeclock;
extern AccelSensor accel;
extern Video video;
extern Arduino_Canvas *bufferCanvas;
extern Haptic haptic;
extern TextMessageService textmessageservice;
extern ExperienceService experienceservice;
extern Steps steps;
extern Timer timer;

#define xmin -700
#define xmax 1500
#define ymin -2500
#define ymax 2500

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
    bool updateTimeLeftNoShow();

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
    bool needssetup;

    unsigned long noMovementTime;

    unsigned long tilttimer;
    int oldtilthour;
    int oldtiltminute;
    float referenceY;
    bool waitForNextReference; // Delay reference update after hour change
    unsigned long lastChangeTime; // Timestamp of the last hour change
    unsigned long lastRepeatTime; // Timestamp of the last auto-repeat
};

#endif // WATCHFACE_H
