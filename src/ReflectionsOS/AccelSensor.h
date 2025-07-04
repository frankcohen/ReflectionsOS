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
#define LOOKAHEAD_MAX   5
#define SAMPLE_RATE     200       // Sample every x milliseconds

#define ACCEL_I2C_ADDR   LIS3DH_DEFAULT_ADDRESS   // 0x18 from Adafruit_LIS3DH

#define CLICKTHRESHHOLD 25    // Strenth of tap to bring back from deep sleep

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    void configureSensorWakeOnMotion();

    void reset();

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
    
    float getXreading();
    float getYreading();
    float getZreading();
  
    String getRecentMessage();
    String getRecentMessage2();

  private:

    Adafruit_LIS3DH lis;

    bool      started;
    bool      runflag;

    int       lookaheadCount;
    unsigned long last;
    int       state;
    unsigned long waittime;

    bool      click;
    unsigned long magtime;
    float     magnow;
    unsigned long magprevtime;
    float     maghistory1;
    float     maghistory2;
    float     maghistory3;

    int shakencount;
    unsigned long shakentime;
    unsigned long shakentime2;
    
    bool      firstpart;
    bool      secondpart;

    bool      _pendingSingle;
    bool      _pendingDouble;
    
    String    myMef;
    String    myMef2;

};

#endif // ACCEL_SENSOR_H
