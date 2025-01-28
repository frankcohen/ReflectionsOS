/*
  Reflections is a hardware and software platform for building entertaining mobile experiences.

  Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS/)
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Reflections board usees an (LIS3DHTR 3-Axis Accelerometer) to
  identify user gestures with their wrists and to wake the
  processor from sleep.

  Recognizes gestures by moving the Reflections board

  See https://github.com/frankcohen/ReflectionsOS/Experiments/AccelerometerLab/ReadMe.md

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Depends on these libraries:
  Adafruit LIS3DH library, https://github.com/adafruit/Adafruit_LIS3DH
  Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
  Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
  PeakDetection, peak and valley detection, https://github.com/leandcesar/PeakDetection

  Datasheet comes with this source code, see:
  1811031937_STMicroelectronics-LIS3DHTR_C15134 Accelerometer.pdf

Sensor and support library details:

getClick() bit definitions:
Bit	Name	   Description
6	  IA	     Interrupt Active: Set to 1 when a click event is detected.
5	  DCLICK	 Double Click: Set to 1 if a double click event is detected.
4	  SCLICK	 Single Click: Set to 1 if a single click event is detected.
3	  Sign	   Sign of Acceleration: Indicates the direction of the click event.
2	  Z	Z-Axis Contribution: Set to 1 if the Z-axis contributed to the click.
1	  Y	Y-Axis Contribution: Set to 1 if the Y-axis contributed to the click.
0	  X	X-Axis Contribution: Set to 1 if the X-axis contributed to the click.

Scaling factor for different ranges:
For LIS3DH_RANGE_2_G (±2g), the scaling factor is 0.001 g per count.
For LIS3DH_RANGE_4_G (±4g), the scaling factor is 0.002 g per count.
For LIS3DH_RANGE_8_G (±8g), the scaling factor is 0.004 g per count.
For LIS3DH_RANGE_16_G (±16g), the scaling factor is 0.008 g per count.

Data rate for different applications:
Low-speed movements (walking): 10–50 Hz., 100 ms delay between reads
Medium-speed movements (hand gestures): 50–100 Hz., 10 ms delay between reads
High-speed movements (vibrations, impacts): 200–400 Hz., 5 ms or less between reads

getClick() bit definitions:
Bit	Name	   Description
6	  IA	     Interrupt Active: Set to 1 when a click event is detected.
5	  DCLICK	 Double Click: Set to 1 if a double click event is detected.
4	  SCLICK	 Single Click: Set to 1 if a single click event is detected.
3	  Sign	   Sign of Acceleration: Indicates the direction of the click event.
2	  Z	Z-Axis Contribution: Set to 1 if the Z-axis contributed to the click.
1	  Y	Y-Axis Contribution: Set to 1 if the Y-axis contributed to the click.
0	  X	X-Axis Contribution: Set to 1 if the X-axis contributed to the click.

Learn how to wake an ESP32-S3 from deep sleep with interrupt movement sensing at 
https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Deep%20Sleep/Deep_Sleep.md

*/

#include "AccelSensor.h"

Adafruit_LIS3DH lis3dh = Adafruit_LIS3DH();

AccelSensor::AccelSensor(){}

void AccelSensor::begin()
{ 
  if ( ! lis3dh.begin( accelAddress ) ) 
  {
    Serial.println( F( "Accelerometer did not start, stopping" ) );
    while (1) yield();
  }

  prevAccel = 15000;
  tapdet = false;
  doubletapdet = false;
  float jerk;
  magtimer = millis();
  debounceDoubleTap = millis();

  range = range1;
  clickThreshold = threshold1;
  powermode = powermode1;
  clickPin = clickPin1;

  debounceTapTime = millis();

  accelThreshold = accelThreshold1;       // for tap detection
  accelThresholdLow = accelThresholdLow1; // for tap detection

  // Set shake detection parameters
  lis3dh.setRange( range );   // 2, 4, 8 or 16 G!

  lis3dh.setDataRate( datarate ); // Set ODR to 100 Hz

  // Set tap detection parameters
  lis3dh.setClick( clickPin1, clickThreshold );  // single click, threshold

  //printSettings();

  delay(2000);

  //printHeader();

  peaklag = peaklag1;
  peakthres = peakthres1;
  peakinfluence = peakinfluence1;

  peakDetection.begin(peaklag, peakthres, peakinfluence);
}

float AccelSensor::getXreading() 
{
  lis3dh.read();
  return lis3dh.x; // Raw X value
}

float AccelSensor::getYreading() 
{
  lis3dh.read();
  return lis3dh.y; // Raw Y value
}

float AccelSensor::getZreading() 
{
  lis3dh.read();
  return lis3dh.z; // Raw Y value
}

bool AccelSensor::tapped()
{
  if ( millis() - debounceDoubleTap > 500 )
  {
    bool result = tapdet;
    tapdet = false;
    return result;
  }
  else
  {
    return false;
  }
}

bool AccelSensor::doubletapped()
{
  bool result = doubletapdet;
  doubletapdet = false;
  return result;
}

// Resets the sensor

void AccelSensor::resetLIS3DH() 
{
  // Read the current value of CTRL_REG5 (0x24)
  uint8_t ctrlReg5Value = 0;
  Wire.beginTransmission(0x18);  // Start communication with LIS3DH (I2C address)
  Wire.write(0x24);  // Register address for CTRL_REG5
  Wire.endTransmission(false);  // Keep the connection active
  Wire.requestFrom(0x18, 1);  // Request 1 byte of data
  if (Wire.available()) {
    ctrlReg5Value = Wire.read();  // Read the current value from CTRL_REG5
  }

  // Set the REBOOT_MEMORY bit (bit 7) to reset the device
  ctrlReg5Value |= 0x80;  // Set the most significant bit to 1 (0x80)

  // Write the new value back to CTRL_REG5 to reboot the sensor
  Wire.beginTransmission(0x18);  // Start communication again
  Wire.write(0x24);  // CTRL_REG5 register address
  Wire.write(ctrlReg5Value);  // Write the updated value
  Wire.endTransmission();  // End transmission

  // Wait for the reset to take effect
  delay(10);
}

/* getEvent()  gets the raw sensor data then convert
   into acceleration values in m/s². These are the values that represent
   how the sensor is moving in 3D space. */

void AccelSensor::readSensor()
{
  lis3dh.read();
  rawX = lis3dh.x;
  rawY = lis3dh.y;
  rawZ = lis3dh.z;
}

void AccelSensor::SimpleRangeFiltering()
{
  if ( millis() - debounceTapTime > 100 )
  {
    readSensor();

    // Calculate the magnitude of the acceleration in all directions
    accelMagnitude = sqrt( ( rawX * rawX ) + ( rawY * rawY ) + ( rawZ * rawZ ) );

    if ( millis() - magtimer < 10 ) return;
    magtimer = millis();

    // Calculate the jerk (rate of change of acceleration)
    jerk = accelMagnitude - prevAccel;
    prevAccel = accelMagnitude;

    if ( millis() - debounceTapTime > 700 )
    {
      if ( ( fabs( jerk ) <= accelThreshold ) && ( fabs( jerk ) > accelThresholdLow ) )
      {
        /*
        String mef = " > ";
        mef += fabs( jerk );
        mef += ", ";
        mef += ( millis() - debounceTapTime );
        Serial.println( mef );
        */

        tapdet = true;
        debounceTapTime = millis();
        debounceDoubleTap = millis();
      }
    }
    else
    {
      if ( ( millis() - debounceTapTime > 250 ) && ( millis() - debounceTapTime < 500 ) && tapdet )
      {
        //Serial.print( "    " );
        //Serial.println( fabs( jerk ) );

        if ( ( fabs( jerk ) <= accelThreshold ) && ( fabs( jerk ) > accelThresholdLow ) )
        {
          doubletapdet = true;
          tapdet = false;
          debounceTapTime = millis();

          /*
          String mef = " >>";
          mef += fabs( jerk );
          mef += ", ";
          mef += ( millis() - mostrecentdoubletaptime );
          Serial.println( mef );
          */
        }
      }
    }
  }

}

void AccelSensor::loop()
{
  SimpleRangeFiltering();
}

