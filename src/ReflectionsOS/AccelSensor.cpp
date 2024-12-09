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
    Serial.println( F( "Could not start accelerometer, stopping" ) );
    while (1) yield();
  }

  // Setup tap detection
  lis.setRange(LIS3DH_RANGE_8_G);   // 2, 4, 8 or 16 G!

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive

  // Adjust CLICKTHRESHOLD for the sensitivity of the 'click' force
  // this strongly depend on the range! for 16G, try 5-10
  // for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
  lis.setClick( 1, CLICKTHRESHOLD );

  delay(100);

  for (int i = 1; i < BUFFER_SIZE; i++) 
  {
    buffer[i].x = 0;
    buffer[i].y = 0;
    buffer[i].z = 0;
  }

  stattap = false;
  statdoubletap = false;
  gotaclick = false;
  testfordouble = false;
  clicktime = millis();
  taptime = millis();

  // lis.setPerformanceMode(LIS3DH_MODE_LOW_POWER);
  Serial.print("Accelerometer performance mode set to: ");
  switch (lis.getPerformanceMode()) {
    case LIS3DH_MODE_NORMAL: Serial.println("Normal 10 bit"); break;
    case LIS3DH_MODE_LOW_POWER: Serial.println("Low Power 8 bit"); break;
    case LIS3DH_MODE_HIGH_RESOLUTION: Serial.println("High Resolution 12 bit"); break;
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
  return statdoubletap;
}

void AccelSensor::loop()
{
  //sampleData();

  if ( millis() - taptime < 100 ) return;
  taptime = millis();

  // if we're within 150 - 600 ms from the single then it's a double and cancel the single, otherwise its a single

  uint8_t click = lis.getClick();
  if (click == 0) return;
  if (! (click & 0x30)) return;
  if (click & 0x10)
  {
    unsigned long mtime = millis() - clicktime;

    //Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
    //Serial.println( mtime );

    if ( ( mtime > 150 ) && ( mtime < 600 ) )
    {
      //Serial.print( "Double click, cancels previous single ");
      //Serial.println( mtime );
      clicktime = millis();
      stattap = false;
      statdoubletap = true;
    }
    else
    {
      //Serial.print( "New single ");
      //Serial.println( mtime );
      clicktime = millis();
      stattap = true;
      statdoubletap = false;
    }
  }

}

