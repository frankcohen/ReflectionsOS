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
#define range1 LIS3DH_RANGE_8_G

// Data rate
// Low-speed movements (walking): 10–50 Hz.
// Medium-speed movements (hand gestures): 50–100 Hz.
// High-speed movements (vibrations, impacts): 200–400 Hz.
#define datarate LIS3DH_DATARATE_100_HZ

// G - Lag, the number of most recent added values considered.
//     Short taps the value is small should be low. Longer movements should use larger values
#define peaklag1 5

// H - Threshhold is how much of a clip upwards or  down to be a peak
// For example, 2 means peaks are 2 times the standard deviation
#define peakthres1 2

// I - Influence boosts the data upwards, it's like a giant sensitivity dail
// Set this between 0.20 and 1.0. Little changes like 0.72 and 0.74 can make
// a big difference
#define peakinfluence1 0.50

// C - Sets the hardware click/tap sensitivity within the R range
#define threshold1 30

// S - Shake threashold
#define shakeThreshold1 9.0

// J - Jerk threashold using range filtering
#define accelThreshold1 2000

// L - Jerk low threadshold using range filtering
#define accelThresholdLow1 1000

// T - restingThreshold  for ignoring small accelerations (e.g., sensor resting)
#define restingThreshold1 0.2

// Sets the power mode, LIS3DH_MODE_NORMAL, LIS3DH_MODE_LOW_POWER, LIS3DH_MODE_HIGH_RESOLUTION
#define powermode1 LIS3DH_MODE_NORMAL

// Sets tap/click to 1 for single click or 2 for double click
#define clickPin1 1

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    void resetLIS3DH();

    bool tapped();
    bool doubletapped();
    bool shaken();

    float getXreading();
    float getYreading();
    float getZreading();

    void SimpleRangeFiltering();
    void recognizeHardwareClicks();  // Tap/click detection using LIS3DH hardware detection
    void readSensor();               // Reads the sensor
    void detectPeaks();
  
  private:
    lis3dh_range_t range;
    lis3dh_mode_t powermode;

    // Acceleration readings
    float accelerationX;
    float accelerationY;
    float accelerationZ;

    // Position readings
    float rawX, rawY, rawZ;

    bool tapdet;
    bool doubletapdet;
    bool trippledet;

    unsigned long debounceTapTime;
    unsigned long debounceDoubleTap;
    unsigned long magtimer;

    int shaketotal;
    int shakecount;
    unsigned long shaketimer;

    float jerk;

    float accelMagnitude;
    float prevAccel;
    int clickThreshold;
    int clickPin;
    float accelThreshold;    // high value for tap detection 
    float accelThresholdLow; // low value for tap detection

    double currentNeutralMeasurement;

    bool started;

    unsigned long reporttimer;
};

#endif // ACCEL_SENSOR_H
