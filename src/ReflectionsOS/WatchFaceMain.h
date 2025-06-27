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
#include "TOF.h"
#include "Steps.h"
#include "TimerService.h"
#include "esp_sleep.h"
#include "GPS.h"
#include "Haptic.h"

#include <PNGdec.h>
#include <Arduino_GFX_Library.h>

#include "WatchFaceBase.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Battery battery;
extern RealTimeClock realtimeclock;
extern AccelSensor accel;
extern TOF tof;
extern Video video;
extern Haptic haptic;
extern TextMessageService textmessageservice;
extern Steps steps;
extern TimerService timerservice;
extern GPS gps;

#define nomov 1500    // Hourglass timeout duration

#define tiltspeed 1500 // Speed to update set-time values

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    
    void setDrawItAll();
    bool isMain();
    bool isSleepy();

    bool okToDeepSleep();
    bool okToLightSleep();
    bool isSleepNow();

    enum Panel { 
      STARTUP, 
      MAIN, 
      DISPLAYING_DIGITAL_TIME, 
      SETTING_DIGITAL_TIME, 
      DISPLAYING_TIMER, 
      SETTING_TIMER,
      DISPLAYING_HEALTH_STATISTICS,
      CONFIRM_TIME, 
      CONFIRM_CLEAR_STEPS, 
      CONFIRM_START_TIMER      
    };
    
  private:
    void startup();
    void main();
    void displayingdigitaltime();
    void settingdigitaltime();
    void displayingtimer();
    void settingtimer();
    void displayinghealthstatistics();
    void changetime();
    void clearsteps();
    void starttimer();
    void resetOnces();
    void printStatus();
    
    void showDisplayMain();

    void updateBlink();
    void updateHoursAndMinutes();
    void updateBattery();
    bool updateTimeLeft();
    void updateTimerNotice();
    void updateGPSmarker();

    bool changing( bool hourflag );

    void drawHourMinute( int hourc, int minutec, bool hourschanging );

    int panel;

    unsigned long sleepytimer;
    unsigned long timetosleep;
    bool sleepnow;

    unsigned long maintimer;

    unsigned long slowman;
    int rowCount;
    
    bool displayUpdateable;

    int oldHour, oldMinute, oldBattery, oldBlink;

    bool onceDigitalTime;
    bool onceHealth;

    unsigned long battimer;
    int batcount;
    int batlev;

    bool blinking;
    unsigned long catFaceTimer;
    int catFaceWait;
    int catFaceIndex;
    bool catFaceDirection;

    bool drawFace;
    bool drawitall;
    bool needssetup;

    unsigned long enoughTimePassedtimer;

    unsigned long noMovementTime;

    int hour;
    int minute;
    unsigned long tilttimer;
    float smallX;
    float largeX;
    float smallY;
    float largeY;
    bool hourschanging;
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);

    bool notificationflag;    

    bool gpsflag;
    int gpsx;
    int gpsy;
    float gpsdirection;

    unsigned long temptimer;

    unsigned long minuteRedrawtimer;


};

#endif // WATCHFACE_H
