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

// Setting the hours/minutes and timer minutes
#define factorLevel 0.3         // How fast to move the neutral zone
#define neutralZoneFactor 0.50  // ±50% of baseline
#define xThreshold 0.4          // Movement threshold for X (horizontal) axis
#define yThreshold 0.4          // Movement threshold for Y (vertical) axis
#define tiltspeed 800           // Speed to chanage

#define nomov 1500    // Hourglass timeout duration

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    
    bool okToSleep();
    bool okToExperience();
    void setDrawItAll();

    enum Panel { 
      STARTUP, MAIN, 
      DISPLAYING_DIGITAL_TIME, SETTING_DIGITAL_TIME, 
      DISPLAYING_TIMER, SETTING_TIMER,
      DISPLAYING_HEALTH_STATISTICS,
      CONFIRM_TIME, CONFIRM_CLEAR_STEPS, CONFIRM_START_TIMER      
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

    unsigned long tilttimer;
    bool hourschanging;
    float baselineY;
    float baselineX;
    int hour;
    int minute;
    bool isOutsideNeutralZone(float value, float baseline, float neutralFactor);
    float adjustBaseline(float baseline, float currentReading, float factor = factorLevel );

    bool notificationflag;    

    bool gpsflag;
    int gpsx;
    int gpsy;

    unsigned long temptimer;

    unsigned long minuteRedrawtimer;
};

#endif // WATCHFACE_H
