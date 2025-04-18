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
#include "TimerService.h"
#include "esp_sleep.h"
#include "GPS.h"

#include <PNGdec.h>
#include <Arduino_GFX_Library.h>

#include "WatchFaceBase.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Battery battery;
extern RealTimeClock realtimeclock;
extern AccelSensor accel;
extern Video video;
extern Haptic haptic;
extern TextMessageService textmessageservice;
extern ExperienceService experienceservice;
extern Steps steps;
extern TimerService timerservice;
extern GPS gps;

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
    bool okToSleep();

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

    void showDisplayMain();

    void updateBlink();
    void updateHoursAndMinutes();
    void updateBattery();
    bool updateTimeLeft();
    void updateTimerNotice();
    void updateGPSmarker();

    bool changing( bool hourflag );

    void drawHourMinute( int currentHour, int currentMinute, bool hourschanging );

    int panel;

    unsigned long maintimer;

    unsigned long slowman;
    int rowCount;
    
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
    float referenceY;
    float referenceX;
    bool waitForNextReference; // Delay reference update after hour change
    bool waitForNextReferenceX; // Delay reference update after hour change

    unsigned long lastChangeTime; // Timestamp of the last hour change
    unsigned long lastChangeTimeX; // Timestamp of the last hour change
    unsigned long lastRepeatTime; // Timestamp of the last auto-repeat
    unsigned long lastRepeatTimeX; // Timestamp of the last auto-repeat

    bool hourschanging;
    unsigned long switchtime;
    int currentNeutralMeasurementY = accel.getYreading() + 15000;
    int currentNeutralMeasurementX = accel.getYreading() + 15000;

    unsigned long timertimer;
    bool notificationflag;    

    bool gpsflag;
    int gpsx;
    int gpsy;

};

#endif // WATCHFACE_H
