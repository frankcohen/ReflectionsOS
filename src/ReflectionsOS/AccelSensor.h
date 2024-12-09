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

#include <Kalman.h>

extern Haptic haptic;
extern Utils utils;
extern LOGGER logger;

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHOLD 22
        
#define DOUBLE_CLICK_WINDOW 500  // Double-click time window in milliseconds

#define BUFFER_SIZE 100         // Number of samples to store (10 seconds if sampling every 100ms)
#define SAMPLE_INTERVAL 200     // Time between samples in milliseconds
#define SINGLE_TAP_TIMEOUT 1000   // Timeout for single tap detection (in milliseconds)

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    bool tapped();
    bool doubletapped();

    // Support methods for other classes
    float getXreading();
    float getYreading();
    float getZreading();

  private:
    void sampleData();

    // Cyclic buffer to store accelerometer readings
    struct AccelReading {
      float x;
      float y;
      float z;
    };
  
    AccelReading buffer[BUFFER_SIZE];
    int bufferIndex;  // Index to track the current position in the buffer

    bool stattap;
    bool statdoubletap;
    unsigned long clicktime;
    bool gotaclick;
    bool testfordouble;
    unsigned long taptime;

    unsigned long lastSampleTime;
};

#endif // ACCEL_SENSOR_H
