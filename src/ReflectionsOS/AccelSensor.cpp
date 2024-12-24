#include "Print.h"
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

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Requires Adafruit LIS3DH library at:
  https://github.com/adafruit/Adafruit_LIS3DH


*/

#include "AccelSensor.h"

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

AccelSensor::AccelSensor(){}

void AccelSensor::begin()
{ 
  bufferIndex = 0;
  
  if ( ! lis.begin( accelAddress ) ) 
  {
    Serial.println( F( "Accelerometer did not start, stopping" ) );
    while (1) yield();
  }

  // Setup tap detection
  lis.setRange( CLICKRANGE );

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

  cctime = millis();

  stattap = false;
  statdoubletap = false;

  /*
  // lis.setPerformanceMode(LIS3DH_MODE_LOW_POWER);
  Serial.print("Accelerometer performance mode set to: ");
  switch (lis.getPerformanceMode()) {
    case LIS3DH_MODE_NORMAL: Serial.println("Normal 10 bit"); break;
    case LIS3DH_MODE_LOW_POWER: Serial.println("Low Power 8 bit"); break;
    case LIS3DH_MODE_HIGH_RESOLUTION: Serial.println("High Resolution 12 bit"); break;
  }
  */
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

// Sample accelerometer data every 100ms and store it in the buffer

void AccelSensor::sampleData() 
{
  // Get the raw data:
  // lis.read();
  // lis.x, lis.y, lis.z

  /* Get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  if ( millis() - lastSampleTime >= SAMPLE_INTERVAL) 
  {
    lastSampleTime = millis();

    // Read accelerometer values

    buffer[bufferIndex].x = event.acceleration.x;
    buffer[bufferIndex].y = event.acceleration.y;
    buffer[bufferIndex].z = event.acceleration.z;

    if ( ( buffer[bufferIndex].x == 0 ) && ( buffer[bufferIndex].y == 0 ) && ( buffer[bufferIndex].z == 0 ) ) return;

    // Move the buffer index and wrap it around if it exceeds BUFFER_SIZE
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  }
}

bool AccelSensor::tapped()
{
  bool result = stattap;
  stattap = false;
  return result;
}

bool AccelSensor::doubletapped()
{
  bool result = statdoubletap;
  statdoubletap = false;
  return result;
}

void AccelSensor::recognizeClick()
{
  if ( millis() - cctime < 150 ) return;
  cctime = millis();

  uint8_t click = lis.getClick();
  if (click == 0) return;
  if (! (click & 0x30)) return;

  // See AccelSensor.h for description of bits

  Serial.print("Click detected (0x");
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

  if (click & 0x10) 
  {
    stattap = true;
    statdoubletap = false;
  }
}

/* Uses accelerometer to adjust hours, minutes, and timer values up or down

Follows this algorithm:

Time Adjustment algorithm implemented in C and C++ as an Arduino 2.3 sketch 
to help me with an accelerometer. You helped me with in the past. This is a new approach. 
So do not use the previous approach. The input to this algorithms is a measurement from 
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

void AccelSensor::loop()
{
  recognizeClick();

  //sampleData();
}

