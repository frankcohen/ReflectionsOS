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

#include "config.h"
#include "secrets.h"

#include "Haptic.h"
#include "Utils.h"
#include "Logger.h"

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <Wire.h>
#include "SD.h"
#include "SPI.h"

#include "Print.h"

extern Haptic haptic;
extern Utils utils;
extern LOGGER logger;

// Default settings

// R - Range default, 2, 4, 8, 16
#define range LIS3DH_RANGE_8_G

// Data rate
// Low-speed movements (walking): 10–50 Hz.
// Medium-speed movements (hand gestures): 50–100 Hz.
// High-speed movements (vibrations, impacts): 200–400 Hz.
#define datarate LIS3DH_DATARATE_100_HZ

// J - Jerk threashold using range filtering
#define accelThreshold 2000

// L - Jerk low threadshold using range filtering
#define accelThresholdLow 600

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    void resetLIS3DH();

    void enableWakeOnMotion();

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
     * Confirmation occurs after the triple-tap window elapses without a third tap.
     */
    bool getDoubleTap();

    /**
     * Returns true once when a triple tap is detected.
     * Detection occurs after three taps are registered within the triple-tap window.
     */
    bool getTripleTap();
    
    float getXreading();
    float getYreading();
    float getZreading();
  
    String getRecentMessage();
    String getRecentMessage2();

  private:
    void dynamicTapDetection();

    void handleTap(unsigned long now);

    float magnitude(const sensors_event_t &e);

    // EMA & threshold parameters
    const float _alpha          = 0.01f;
    const float _thresholdFactor= 4.0f;

    // Tap timing windows
    const unsigned long _minTapInterval = 20;    // ms bounce filter
    const unsigned long _doubleWindow   = 1200;  // ms for 2 taps
    const unsigned long _tripleWindow   = 2400;  // ms for 3 taps

    // Reset & summary timing
    const unsigned long _resetInterval   = 5000; // ms
    const unsigned long _summaryInterval = 1000; // ms

    // Running statistics
    float emaMean;
    float emaVar;
    bool  aboveThreshold;
    unsigned long lastDetectedTime;

    // Tap history timestamps
    unsigned long tapHist[3];

    bool          _pendingSingle;
    bool          _pendingDouble;
    bool          _pendingTriple;
    unsigned long _singleTime;
    unsigned long _doubleTime;
    unsigned long _tripleTime;

    // Counters & timers
    unsigned long singleCount;
    unsigned long doubleCount;
    unsigned long tripleCount;
    unsigned long lastResetTime;
    unsigned long lastSummaryTime;

    // Debug stats
    float lastMag;
    float lastStdDev;
    float lastThreshold;

    bool started;
    bool runflag;
    
    String myMef;
    String myMef2;
};

#endif // ACCEL_SENSOR_H
