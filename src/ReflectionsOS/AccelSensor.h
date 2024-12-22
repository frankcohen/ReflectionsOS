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


// Range defines sensitivity of the sensor
// 2, 4, 8 or 16 G

#define CLICKRANGE LIS3DH_RANGE_8_G

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80

#define CLICKTHRESHOLD 25     // Got to 25 by trial and error, no big science behind it
 
/*
getClick() bit definitions:
Bit	Name	   Description
6	  IA	     Interrupt Active: Set to 1 when a click event is detected.
5	  DCLICK	 Double Click: Set to 1 if a double click event is detected.
4	  SCLICK	 Single Click: Set to 1 if a single click event is detected.
3	  Sign	   Sign of Acceleration: Indicates the direction of the click event.
2	  Z	Z-Axis Contribution: Set to 1 if the Z-axis contributed to the click.
1	  Y	Y-Axis Contribution: Set to 1 if the Y-axis contributed to the click.
0	  X	X-Axis Contribution: Set to 1 if the X-axis contributed to the click.
*/

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
    void recognizeClick();

    // Cyclic buffer to store accelerometer readings
    struct AccelReading {
      float x;
      float y;
      float z;
    };
  
    unsigned long lastSampleTime;
    AccelReading buffer[BUFFER_SIZE];
    int bufferIndex;  // Index to track the current position in the buffer

    unsigned long cctime;
    bool stattap;
    bool statdoubletap;
};

#endif // ACCEL_SENSOR_H
