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

#define nomov 1500                  // Hourglass timeout duration
#define sleepyTime (60 * 1000 * 6)  // 6 minutes until feeling sleepy
#define sleepyTime2 (60 * 1000 * 8) // 8 minutes to fall alseep
#define tiltspeed 1200              // Speed to update set-time values

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    
    void setDrawItAll();
    bool isMain();
    bool isSleepy();
    bool goToSleep();
    void clearSleepy();
    
    enum Panel { 
      STARTUP, 
      MAIN, 
      
      DISPLAYING_TIME, 
      SETTING_TIME,
      CONFIRM_SETTING_TIME,

      DISPLAYING_MOVES,
      CONFIRM_CLEAR_MOVES, 

      DISPLAYING_TIMER, 
      SETTING_TIMER,
      CONFIRM_START_TIMER
    };
    
  private:
    void changeTo( int panelnum, bool setup, String videoname );

    void startup();
    void main();

    void displaytime();
    void settingtime();
    void confirmsettingtime();

    void displayingmoves();
    void confirmclearmoves();

    void displayingtimer();
    void settingtimer();
    void confirmstarttimer();
  
    void printStatus();
    
    void showDisplayMain();

    void updateBlink();
    void updateHoursAndMinutes();
    void updateBattery();
    bool updateTimeLeft();
    void updateTimerNotice();
    void updateGPSmarker();

    bool changing( bool hourflag );

    void drawHourMinute( int hourc, int minutec );

    int panel;

    unsigned long maintimer;

    unsigned long slowman;
    int rowCount;
    
    bool movesflag;

    bool displayUpdateable;

    int oldHour, oldMinute, oldBattery, oldBlink;

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

    unsigned long sleepyTimer;
    unsigned long sleepyTimer2;

    unsigned long movesOld;

};

#endif // WATCHFACE_H
