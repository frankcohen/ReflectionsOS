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

#define AccelCalN 59

#define WAIT_TIME       3000
#define LOOKAHEAD_MAX   8
#define SAMPLE_RATE     50       // Sample every x milliseconds
#define CLICKTHRESHHOLD 25

#define ACCEL_I2C_ADDR   LIS3DH_DEFAULT_ADDRESS   // 0x18 from Adafruit_LIS3DH

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    void resetLIS3DH();

    void configureSensorWakeOnMotion();

    void reset();

    void setStatus( bool running );
    bool getStatus();

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
    void computeThreshold(const double *vals, int n, double &outMedian, double &outMad);
    void runSimulation();
    void fillValues();

    Adafruit_LIS3DH lis;

    int      lookaheadCount;
    int      firstIdx;
    double   firstMag;
    double   threshold;
    unsigned long sampleIndex;
    unsigned long last;
    int      state;
    unsigned long waittime;

    bool     _pendingSingle;
    bool     _pendingDouble;

    bool started;
    bool runflag;
    
    String myMef;
    String myMef2;

};

#endif // ACCEL_SENSOR_H
