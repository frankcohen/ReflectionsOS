/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef ACCEL_SENSOR_H
#define ACCEL_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "secrets.h"

#include "Utils.h"
#include "Logger.h"

extern Utils utils;
extern LOGGER logger;

#define WAIT_TIME       2000
#define SAMPLE_RATE     200       // Sample every x milliseconds

#define ACCEL_I2C_ADDR   LIS3DH_DEFAULT_ADDRESS   // 0x18 from Adafruit_LIS3DH

#define CLICKTHRESHHOLD 25    // Strenth of tap to bring back from deep sleep

#define SHAKEN_COUNT 2        // Big movements to signal shaken gesture

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    void configureSensorWakeOnMotion();

    void setStatus( bool running );
    bool getStatus();

    bool isShaken();

    /**
     * Returns true once when a confirmed single tap is detected.
     * Confirmation occurs after the double-tap window elapses without a second tap.
     */
    bool getSingleTap();

    /**
     * Returns true once when a confirmed double tap is detected.
     */
    bool getDoubleTap();

    void resetTaps();
    bool getSingleTapNoClear();
    bool getDoubleTapNoClear();
    
    float getXreading();
    float getYreading();
    float getZreading();
  
    String getRecentMessage();
    String getRecentMessage2();

  private:

    void handleClicks();
    
    Adafruit_LIS3DH lis;

    bool      started;
    bool      runflag;

    bool      firstpart;
    unsigned long firstClickTime;
    unsigned long minClickTime;
    unsigned long waittime;
    bool      _pendingSingle;
    bool      _pendingDouble;

    int shakencount;
    unsigned long shakentime;
    unsigned long shakentime2;

    unsigned long magtime;
    float magnow;

    unsigned long last;        
    String    myMef;
    String    myMef2;

};

#endif // ACCEL_SENSOR_H
