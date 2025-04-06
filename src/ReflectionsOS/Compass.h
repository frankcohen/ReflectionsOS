/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Provides compas/magnetometer readings from the MMC5603NJ device
 
 Datasheet on the sensor:
 - Superior Dynamic Range and Accuracy:
 - ±30 G FSR
 - 20bits operation mode
 - 0.0625mG per LSB resolution
 - 2 mG total RMS noise
 - Enables heading accuracy of 1º
 - Sensor true frequency response up to 1KHz

Pretty cood and tiny chip:
  Full-Scale Range (FSR): ±30 G (GigaGauss), which is ±30,000 mG (since 1 G = 1000 mG).
  Resolution: 20-bit operation mode.
  Resolution per LSB (Least Significant Bit): 0.0625 mG per LSB.

  - ±30 G FSR means the magnetometer is capable of measuring magnetic fields in the range of ±30,000 mG (or ±30 G).

  - 0.0625 mG per LSB is the smallest measurable unit per ADC reading, which corresponds to the resolution of the sensor.

  - 20-bit resolution means the sensor can output a value between 0 and 2^20 (1,048,576 discrete values). Each of these values represents 0.0625 mG of change in the magnetic field.

*/

#ifndef _COMPASS_
#define _COMPASS_

#include "Arduino.h"
#include "config.h"
#include "secrets.h"
#include <Adafruit_MMC56x3.h>

#define MMC5603NJ_ID 0x00     // Internal ID for the magnetometer as found in the WHO_AM_I register
#define MMC5603NJ_ID_Alt 0x10 // Alternate for certain chips

// The sensor I²C address is 0x30.
#define SENSOR_ADDRESS     0x30

// Counters placement of device on board
#define CompassOffsetAdjustment -150

#define DECLINATION -0.08387 // declination (in degrees) in Silicon Valley, California USA

class Compass {
  public:
    Compass();
    void begin();
    void loop();
    bool test();

    String decodeHeading( float measured_angle );
    int decodeHeadingVal( float measured_angle );

    void callibrate();
    float getHeading();
    void read_XYZ();

  private:
    Adafruit_MMC5603 mag;

    // Current calibrated sensor readings (in physical units)
    float X, Y;

    // Calibration values for X and Y: running minimum, maximum, and midpoint
    float Max[2], Min[2], Mid[2];

    // Timer for periodic actions
    unsigned long ctimer;

    // Flag indicating if initialization was successful
    boolean started;
};

#endif // _COMPASS_
