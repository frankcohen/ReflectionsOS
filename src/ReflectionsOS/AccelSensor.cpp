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

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

AccelSensor::AccelSensor(){}

void AccelSensor::begin()
{ 
  if ( ! lis.begin( accelAddress ) ) 
  {
    Serial.println( F( "Accelerometer did not start, stopping" ) );
    while (1) yield();
  }

  alabtimer = millis();
  magtimer = millis();
  shake = false;
  rowCount = 0;
  lastTapTime = millis();
  cctime = millis();
  prevAccel = 15000;
  debouncePeakTap = millis();
  peakTimer = millis();
  peakTap = false;
  tapdet = false;

  alpha = 0.1;            // Low-pass filter factor (adjustable)
  initialAccelX = 0;
  initialAccelY = 0;
  initialAccelZ = 0;
  lastTime = millis();
  restFlag = true;
  peakcnt = 0;

  range = range1;
  clickThreshold = threshold1;
  powermode = powermode1;
  clickPin = clickPin1;

  debounceTapTime = millis();

  accelThreshold = accelThreshold1;       // for tap detection
  accelThresholdLow = accelThresholdLow1; // for tap detection
  shakeThreshold = shakeThreshold1;       // for shake detection
  restingThreshold = restingThreshold1;   // for resting detection

  // Set shake detection parameters
  lis3dh.setRange( range );   // 2, 4, 8 or 16 G!

  lis3dh.setDataRate( datarate ); // Set ODR to 100 Hz

  // Set tap detection parameters
  lis3dh.setClick( clickPin1, clickThreshold );  // single click, threshold

  printSettings();

  delay(2000);

  printHeader();

  peaklag = peaklag1;
  peakthres = peakthres1;
  peakinfluence = peakinfluence1;

  peakDetection.begin(peaklag, peakthres, peakinfluence);
  
  // Setup tap detection
  lis.setRange( CLICKRANGE );
  range = CLICKRANGE;

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive

  lis.setClick( 1, CLICKTHRESHOLD );

  delay(100);

  for (int i = 1; i < BUFFER_SIZE; i++) 
  {
    buffer[i].x = 0;
    buffer[i].y = 0;
    buffer[i].z = 0;
  }

  rowCount = 0;

  cctime = millis();
  ccinterval = 500;

  slowman = millis();

  stattap = false;
  statshake = false;
  statdoubletap = false;

  debounceTapTime = millis();

  accelThreshold = accelThreshold1; // for tap detection
  accelThresholdLow = accelThresholdLow1; // for tap detection
  shakeThreshold = shakeThreshold1; // for shake detection
  restingThreshold = restingThreshold1; // Threshold for ignoring small accelerations (e.g., sensor resting)

  initialAccelX = 0;
  initialAccelY = 0;
  initialAccelZ = 0;
  lastTime = millis();

  restFlag = true;
}

// Print settings to the Serial Monitor

void AccelSensor::printSettings()
{
  Serial.println( " " );
  Serial.println( " " );
  Serial.println( "AccelerometerLab settings" );

  Serial.print( "Range " );
  Serial.println( range );
  Serial.print( "Shake threashold " );
  Serial.println( shakeThreshold );
  Serial.print( "Jerk threashold " );
  Serial.println( accelThreshold );
  Serial.print( "Jerk threashold low " );
  Serial.println( accelThresholdLow ); 

  lis3dh.setPerformanceMode( powermode );
  Serial.print("Accelerometer performance mode set to: ");
  switch (lis3dh.getPerformanceMode()) {
    case LIS3DH_MODE_NORMAL: Serial.println("Normal 10 bit"); break;
    case LIS3DH_MODE_LOW_POWER: Serial.println("Low Power 8 bit"); break;
    case LIS3DH_MODE_HIGH_RESOLUTION: Serial.println("High Resolution 12 bit"); break;
  }
}

// Function to print the header

void AccelSensor::printHeader() 
{
  Serial.println("Enter R for range, A low-pass filter, C click/tap sensitivity, S shake threshold, J jerk threshold,");
  Serial.println( "X to reset the device, PeakDetection G lag, H threashold, I influence, F peak jerk threashold" );

  Serial.print("Example inputs: R ");

  if (range == LIS3DH_RANGE_2_G) 
  {
    Serial.print("2");
  } 
  if (range == LIS3DH_RANGE_4_G) 
  {
    Serial.print("4");
  } 
  if (range == LIS3DH_RANGE_8_G) 
  {
    Serial.print("8");
  } 
  if (range == LIS3DH_RANGE_16_G) 
  {
    Serial.print("16");
  }

  Serial.print( ", A ");
  Serial.print( alpha );

  Serial.print( ", C ");
  Serial.print( clickThreshold );

  Serial.print( ", S ");
  Serial.print( shakeThreshold );

  Serial.print( ", J ");
  Serial.print( accelThreshold );

  Serial.print( ", L ");
  Serial.print( accelThresholdLow );

  Serial.print( ", T ");
  Serial.print( restingThreshold );

  Serial.print( ", G ");
  Serial.print( peaklag );
  Serial.print( ", H ");
  Serial.print( peakthres );
  Serial.print( ", I ");
  Serial.print( peakinfluence );
  Serial.print( ", F ");
  Serial.println( jerkthreshold );

  Serial.println(" Acceleration                Raw                                                  PeakDetection");
  Serial.println(" X        Y       Z          X     Y      Z        Magni   Jerk  RestC Shake Tap  Filter Peak");
}

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

/* Handles commands entered through the Serial Monitor */

void AccelSensor::handleCommands()
{
  // Check if there's any data available in the Serial Monitor
  if (Serial.available() > 0) 
  {
    char input = Serial.read();  // Read the first character
    String value = "";           // To store the input value

    // Wait for the rest of the input (number)
    while (Serial.available() > 0) 
    {
      value += (char)Serial.read();  // Read the input value
      delay(10);  // Small delay to allow the Serial buffer to fill
    }

    if (input == 'X' || input == 'x') 
    {
      resetLIS3DH();
      Serial.println( "Reset accelerometer" );
      delay( 2000 );
    }

    // Handle the input based on the character received
    if (input == 'R' || input == 'r') 
    {
      // Set the range value
      int newRange = value.toInt();  // Convert the value to an integer
      
      if (newRange == 2 || newRange == 4 || newRange == 8 || newRange == 16) 
      {
        if (newRange == 2) 
        {
          lis3dh.setRange(LIS3DH_RANGE_2_G);
          range = LIS3DH_RANGE_2_G;
          Serial.println("Range set to: LIS3DH_RANGE_2_G");
          delay(2000);
        } 
        else if (newRange == 4) 
        {
          lis3dh.setRange(LIS3DH_RANGE_4_G);
          range = LIS3DH_RANGE_4_G;
          Serial.println("Range set to: LIS3DH_RANGE_4_G");
          delay(2000);
        } 
        else if (newRange == 8) 
        {
          lis3dh.setRange(LIS3DH_RANGE_8_G);
          range = LIS3DH_RANGE_8_G;
          Serial.println("Range set to: LIS3DH_RANGE_8_G");
          delay(2000);
        } 
        else if (newRange == 16) 
        {
          lis3dh.setRange(LIS3DH_RANGE_16_G);
          range = LIS3DH_RANGE_16_G;
          Serial.println("Range set to: LIS3DH_RANGE_16_G");
          delay(2000);
        }
      } 
      else 
      {
        Serial.println("Invalid input for Range. Please enter 2, 4, 8, or 16.");
        delay(2000);
      }
    }
    
    else if (input == 'S' || input == 's') 
    {
      // Set the shake threshold
      float newShakeThreshold = value.toFloat();  // Convert the value to a float
      if (newShakeThreshold > 0) 
      {
        shakeThreshold = newShakeThreshold;
        Serial.print("Shake threshold set to: ");
        Serial.println(shakeThreshold);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Shake threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'T' || input == 't') 
    {
      // Set the shake resting threshold
      float newResting = value.toFloat();  // Convert the value to a float
      if (newResting > 0) 
      {
        restingThreshold = newResting;
        Serial.print("Shake resting threshold set to: ");
        Serial.println( restingThreshold );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Resting Thhreshold filter. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'C' || input == 'c') 
    {
      // Set the click threshold
      float newClickThreshold = value.toFloat();  // Convert the value to a float
      if ( newClickThreshold > 0 ) 
      {
        clickThreshold = newClickThreshold;
        lis3dh.setClick( clickPin, clickThreshold);  // Set new click threshold
        Serial.print("Click threshold set to: ");
        Serial.println(clickThreshold);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Click threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'J' || input == 'j') 
    {
      // Set the acceleration threshold for shake detection
      float newAccelThreshold = value.toFloat();  // Convert the value to a float
      if (newAccelThreshold > 0) 
      {
        accelThreshold = newAccelThreshold;
        Serial.print("Acceleration threshold set to: ");
        Serial.println(accelThreshold);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Acceleration threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'L' || input == 'j') 
    {
      // Set the acceleration threshold for shake detection
      float newAccelThresholdLow = value.toFloat();  // Convert the value to a float
      if (newAccelThresholdLow > 0) 
      {
        accelThresholdLow = newAccelThresholdLow;
        Serial.print("Acceleration low threshold set to: ");
        Serial.println(newAccelThresholdLow);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Acceleration Low threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'G' || input == 'g') 
    {
      // Set the acceleration threshold for shake detection
      float newPeakLag = value.toFloat();  // Convert the value to a float
      if (newPeakLag > 0) 
      {
        peaklag = newPeakLag;
        peakDetection.begin(peaklag, peakthres, peakinfluence);
        Serial.print("PeakDetection lag threshold set to: ");
        Serial.println( newPeakLag );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for PeakDetection lag threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'H' || input == 'h') 
    {
      // Set the acceleration threshold for shake detection
      float newPT = value.toFloat();  // Convert the value to a float
      if (newPT > 0) 
      {
        peakthres = newPT;
        peakDetection.begin(peaklag, peakthres, peakinfluence);
        Serial.print("PeakDetection threshold set to: ");
        Serial.println( peakthres );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for PeakDetection lag threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'I' || input == 'i') 
    {
      // Set the acceleration threshold for shake detection
      float newPeakInflu = value.toFloat();  // Convert the value to a float
      if (newPeakInflu > 0) 
      {
        peakinfluence = newPeakInflu;
        peakDetection.begin(peaklag, peakthres, peakinfluence);
        Serial.print("PeakDetection peak influence set to: ");
        Serial.println( peakinfluence );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for PeakDetection lag threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'F' || input == 'f') 
    {
      // Set the jerk threshold peak recognition
      float newJT = value.toFloat();  // Convert the value to a float
      if (newJT > 0) 
      {
        jerkthreshold = newJT;
        Serial.print("Jerkthreshold set to: ");
        Serial.println( jerkthreshold );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for jerkthreshold. Please enter a positive value.");
        delay(2000);
      }
    }
  }
}

float AccelSensor::getXreading() 
{
  lis.read();
  return lis.x; // Raw X value
}

float AccelSensor::getYreading() 
{
  lis.read();
  return lis.y; // Raw Y value
}

float AccelSensor::getZreading() 
{
  lis.read();
  return lis.z; // Raw Y value
}

bool AccelSensor::tapped()
{
  bool result = stattap;
  stattap = false;
  return result;
}

bool AccelSensor::shaken()
{
  bool result = statshake;
  statshake = false;
  return result;
}

bool AccelSensor::doubletapped()
{
  bool result = statdoubletap;
  statdoubletap = false;
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

// Print the header to serial monitor

String AccelSensor::printHeader() 
{
  //Serial.println(" Acceleration                Raw                   ");
  //Serial.println(" X        Y       Z          X     Y      Z        Magni   Jerk  RestC  Shake  Tap");
  return " X        Y       Z          X     Y      Z        Magni   Jerk  RestC  Shake  Tap";
}

// Prints values to Serial Monitor

String AccelSensor::printValues()
{
  sprintf(output, "%7.2f %7.2f %7.2f | %6.0f %6.0f %6.0f | %8.0f %6.0f %6.0f %4.2f | %3d %3d",
        accelerationX, accelerationY, accelerationZ,
        rawX, rawY, rawZ,
        accelMagnitude, jerk, jerk + 15000, restingThreshold,
        statshake, tapdet
        );
  
  return output;

  //return sprintf(output, "%6.0f %3d %3d", jerk, statshake, tapdet );

  //Serial.println(output);
}

/* Uses accelerometer to adjust hours, minutes, and timer values up or down

Follows this algorithm:

Time Adjustment algorithm implemented in C and C++ as an Arduino 2.3 sketch 
to help me with an accelerometer. The input to this algorithms is a measurement from 
the accelerometer. The input will always be within the range of 1 to 30,000, always positive. 
The first input is always 15,000. Measurements are taken every 300 milliseconds. The first 
input establishes a Neutral Zone. This zone begins 10% below and ends 10% above the 
measurement. Subsequent measurements that fall inside the Neutral Zone make no change 
to the time. Immediately below the Neutral Zone is an Increase Zone. Subsequent measurements 
in the Increase Zone increase by 1 the current Time value. When the Time value reaches 13 is 
loops back to 1. When the Time value changes the code waits 500 milliseconds before the next 
subsequent measurement. Immediately above the Neutral Zone is a Decrease Zone. Subsequent 
measurements in the Decrease Zone decrease by 1 the current Time value. When the Time value 
reaches 0 is loops back to 12. When the Time value changes the code waits 500 milliseconds 
before the next subsequent measurement. If the Subsequent measurement is in the Decrease Zone 
after previously being in the Increase Zone, the no decrease of time is made and the location 
of the Neutral Zone recalculates to the new measurement. If the Subsequent measurement is in 
the Increase Zone after previously being in the Decrease Zone, the no increase of time is made 
and the location of the Neutral Zone recalculates to the new measurement. 

*/

void AccelSensor::adjustTime(int measurement, int minTime, int maxTime) 
{
  // Calculate Neutral Zone boundaries based on the currentNeutralMeasurement
  int lowerBound = currentNeutralMeasurement - (currentNeutralMeasurement / 10);
  int upperBound = currentNeutralMeasurement + (currentNeutralMeasurement / 10);
  
  // Neutral Zone: No change in Time value
  if ( ( measurement >= lowerBound ) && ( measurement <= upperBound ) ) 
  {
    return;
  }

  // Increase Zone: Below the Neutral Zone
  if ( measurement < lowerBound ) 
  {
    currentTime = (currentTime == maxTime) ? minTime : currentTime + 1;
    currentNeutralMeasurement = currentNeutralMeasurement - 100;
    return;
  }
  
  // Decrease Zone: Above the Neutral Zone
  if ( measurement > upperBound ) 
  {
    currentTime = (currentTime == minTime) ? maxTime : currentTime - 1; 
    currentNeutralMeasurement = currentNeutralMeasurement + 100;
    return;
  }
  
  return;
}

int AccelSensor::getTime()
{
  return currentTime;
}

void AccelSensor::setTime( int mytime )
{
  currentTime = mytime;
}

void AccelSensor::recognizeHardwareClicks()
{
  if ( millis() - cctime > 2000 )
  {
    cctime = millis();

    uint8_t click = lis3dh.getClick();
    if ( click == 0 ) return;
    if ( ! ( click & 0x30 ) ) return;

    Serial.print( "Click detected (0x" );
    Serial.print( click, HEX ); 
    Serial.print( " " );

    for (int b = 7; b >= 0; b--)
    {
      Serial.print(bitRead(click, b));
    }  
    Serial.print("): ");

    if (click & 0x01) {
      Serial.print("X, ");
    }
    if (click & 0x02) {
      Serial.print("Y, ");
    }
    if (click & 0x04) {
      Serial.print("Z, ");
    }
    if (click & 0x08) {
      Serial.print("Sign, ");
    }
    if (click & 0x10) {
      Serial.print("Single click, ");
    }
    if (click & 0x20) {
      Serial.print("Double click ");
    }
    Serial.println(" ");
  }
}

/* getEvent()  gets the raw sensor data then convert
   into acceleration values in m/s². These are the values that represent
   how the sensor is moving in 3D space. */

void AccelSensor::readSensor()
{
  sensors_event_t event;
  lis3dh.getEvent(&event);

  accelerationX = event.acceleration.x;
  accelerationY = event.acceleration.y;
  accelerationZ = event.acceleration.z;
  
  if ( restFlag )
  {
    initialAccelX = accelerationX;
    initialAccelY = accelerationY;
    initialAccelZ = accelerationZ;
    restFlag = false;
  }

  lis3dh.read();
  rawX = lis3dh.x;
  rawY = lis3dh.y;
  rawZ = lis3dh.z;

  /* The read() method gives the raw acceleration values in ADC units or 
    counts (not in m/s²). These are unprocessed values from the sensor's internal
    registers. To convert them to physical units (like m/s² or g), the following
    applies a scaling factor based on the accelerometer's configuration (i.e., the set 
    range, such as ±2g, ±4g, etc.) */

  float scaleFactor = 16384.0; // For ±2g range
  if (range == LIS3DH_RANGE_2_G) scaleFactor = 16384.0;
  if (range == LIS3DH_RANGE_4_G) scaleFactor = 8192.0;
  if (range == LIS3DH_RANGE_8_G) scaleFactor = 4096.0;
  if (range == LIS3DH_RANGE_16_G) scaleFactor = 2048.0;

  accelX_g = rawX / scaleFactor;
  accelY_g = rawY / scaleFactor;
  accelZ_g = rawZ / scaleFactor;

  accelX_ms2 = accelX_g * 9.80665;
  accelY_ms2 = accelY_g * 9.80665;
  accelZ_ms2 = accelZ_g * 9.80665;
}

void AccelSensor::loop()
{
  handleCommands();   // Process commands through the Serial Monitor

  // Check for tap/click detection

  recognizeHardwareClicks();  // Tap/click detection using LIS3DH hardware detection
  readSensor();               // Reads the sensor
  detectShake();              // Detect shake

void AccelSensor::detectShake()
{
  if ( millis() - lastTime >= 2000 ) 
  {
    lastTime = millis();
    restFlag = true;

    // Calculate the difference between initial and final accelerations
    thresholdX = abs(accelerationX - initialAccelX);
    thresholdY = abs(accelerationY - initialAccelY);
    thresholdZ = abs(accelerationZ - initialAccelZ);

    // Check if the accelerometer is at rest (small values close to zero)
    if ( ( abs(accelerationX) < restingThreshold ) 
      && ( abs(accelerationY) < restingThreshold )
      && ( abs(accelerationZ) < restingThreshold ) ) 
    {
      // If the sensor is resting, ignore the shake detection
      Serial.println("Resting, no shake detected.");
    } 
    else 
    {
      // Check if the difference exceeds the shake threshold
      if ( ( thresholdX > shakeThreshold )
         || ( thresholdY > shakeThreshold )
         || ( thresholdZ > shakeThreshold ) )
      {
        shake = true;
      } 
      else 
      {
        shake = false;
      }
    }
  }
}

void AccelSensor::





  // Calculate the magnitude of the acceleration in all directions
  accelMagnitude = sqrt( ( rawX * rawX ) + ( rawY * rawY ) + ( rawZ * rawZ ) );

  if ( millis() - magtimer < 10 ) return;
  magtimer = millis();

  // Calculate the jerk (rate of change of acceleration)
  float jerk = accelMagnitude - prevAccel;
  prevAccel = accelMagnitude;

  // Check for tap (sharp increase followed by quick return)

  if ( millis() - debounceTapTime > 1500 )
  {
    if ( millis() - lastTapTime > 50 )
    {
      lastTapTime = millis();

      if ( ( fabs( jerk ) <= accelThreshold ) && ( fabs( jerk ) > accelThresholdLow ) )
      {        
        tapdet = true;
        debounceTapTime = millis();
      }
    }
  }

  double djerk = jerk;

  if ( fabs( djerk ) > jerkthreshold )
  {
    peakDetection.add( djerk );
    peakcnt++;
  }

  // Show the results periodically

  if ( millis() - alabtimer < 1000 ) return;
  alabtimer = millis();

  peak = peakDetection.getPeak();
  filtered = peakDetection.getFilt();

  // Every 10 rows, reprint the header
  rowCount++;
  if (rowCount >= 20) {
    rowCount = 0;  // Reset the counter
    printHeader();  // Reprint the header
  }

  char output[200];  // Buffer to hold the formatted string

  sprintf(output, "%7.2f %7.2f %7.2f | %6.0f %6.0f %6.0f | %8.0f %6.0f %4.2f | %3d %3d | %4.0f  % d",
        accelerationX, accelerationY, accelerationZ,
        rawX, rawY, rawZ,
        accelMagnitude, jerk, restingThreshold,
        shake, tapdet,
        filtered, peak
        );

  /*
  // Suitable for cvs import to a spreadsheet
  sprintf(output, "%7.2f,%7.2f,%7.2f,%6.0f,%6.0f,%6.0f,%8.0f,%6.0f,%4.2f,%3d,%3d,%4.0f,%d, %lu",
        accelerationX, accelerationY, accelerationZ,
        rawX, rawY, rawZ,
        accelMagnitude, jerk, restingThreshold,
        shake, tapdet,
        filtered, peak,
        peakcnt 
        );
  */

  Serial.println(output);

  peakTap = false;
  tapdet = false;

  delay(500);



}

