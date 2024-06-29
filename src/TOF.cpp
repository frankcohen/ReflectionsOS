/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
*/

#include "TOF.h"

TOF::TOF(){}

void TOF::begin()
{ 
  started = false;

  Serial.println( F( "Initializing TOF sensor" ) );

  if ( tofSensor.begin( 0x29, Wire ) == false )
  {
    Serial.println( F("TOF sensor not found") );
    return;
  }

  tofSensor.setResolution(8*8); //Enable all 64 pads
  
  imageResolution = tofSensor.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  tofSensor.setRangingFrequency(15); // 15Hz for low latency

  tofSensor.startRanging();

  lastPollTime = millis();

  cancelDetected = false;
  cancelGestureTimeout = millis();

  started = true;
}

int TOF::getXFingerPosition()
{
  if ( tofSensor.isDataReady() == false ) return 0;

  if ( ! tofSensor.getRangingData(&measurementData) ) return 0;    //Read distance data into array

  int pointx = 0;
  int pointy = 0;
  int pointd = 0;

  for (int x = 0; x < 8; x++)
  {
    for ( int y = 0; y < 8; y++)
    {
      int d = measurementData.distance_mm[ y + ( x * 8 ) ];
      if ( ( d > pointd ) && ( d < maxdist ) )
      {
        pointx = x;
        pointy = y;
        pointd = d;

        pointx++;
        pointy++;
      }
    }
  }

  return pointx;
}

bool TOF::tofStatus()
{
  return started;
}

bool TOF::test()
{
  return started;  
}

/* Pretty print sensor measurements */

void TOF::printTOF()
{
  if ( ! started ) return;

  //Poll sensor for new data
  if ( tofSensor.isDataReady() == true )
  {
    if ( tofSensor.getRangingData( &measurementData ) ) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for ( int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth )
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print( F( "\t" ) );
          Serial.print(measurementData.distance_mm[x + y]);
        }
        Serial.println();
      }
      Serial.println();
    }
  }
}

bool TOF::cancelGestureDetected()
{
  return cancelDetected;
}

void TOF::checkForCancelGesture() 
{
  if ( tofSensor.isDataReady() )
  {
    tofSensor.getRangingData(&measurementData);

    int closeReadingsCount = 0;

    for ( int i = 0; i < 64; i++ ) 
    {
      int distance = measurementData.distance_mm[ i ];

      // Check if the detected distance is within the 1-2 inch range
      if (distance > 0 && distance < detectionThreshold) 
      {
        closeReadingsCount++;
      }
    }

    // If the number of close readings exceeds the majority threshold, register a cancel gesture
    if (closeReadingsCount > majorityThreshold)
    {
      Serial.println("Cancel gesture detected");
      cancelDetected = true;
      cancelGestureTimeout = millis();
    }
  }
}

int TOF::getNextGesture()
{
  if ( cancelDetected )
  {
    cancelDetected = false;
    return 1;
  }

  return 0;
}

/* Clears older gestures, allowing new ones to register */

void TOF::removeExpiredGestures()
{
  unsigned long timely = millis();

  if ( timely - cancelGestureTimeout > 2000 )
  {
    cancelDetected = false;
  }

  // Add additional timeouts here

}

void TOF::loop()
{
  //removeExpiredGestures();

  if ( millis() - lastPollTime >= 2000 ) 
  {
    lastPollTime = millis();
    checkForCancelGesture();
  }

}
