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

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

TOF::TOF(){}

void TOF::begin()
{ 
  started = false;
  lastPollTime = millis();
  nextCancelflag = false;

  // Create the task for sensor initialization
  xTaskCreate(
    TOF::sensorInitTaskWrapper,      // Function that implements the task
    "SensorInitTask",    // Text name for the task
    10000,               // Stack size in words, not bytes
    this,                // Parameter passed into the task
    1,                   // Priority at which the task is created
    &sensorInitTaskHandle // Handle to the created task
  );
}

void TOF::sensorInitTaskWrapper( void * parameter ) 
{
  // Cast the parameter to a VL53L5CX_Sensor pointer
  TOF *self = static_cast<TOF*>(parameter);
  // Call the instance task function
  self->sensorInitTask();
}

void TOF::sensorInitTask() 
{
  if ( tofSensor.begin( 0x29, Wire ) ) 
  {
    logger.info( F( "TOF sensor started" ) );

    cancelDetected = false;
    cancelGestureTimeout = millis();
    lastPollTime = millis();

    tofSensor.setResolution(8*8); //Enable all 64 pads
    
    imageResolution = tofSensor.getResolution();  //Query sensor for current resolution - either 4x4 or 8x8
    imageWidth = sqrt(imageResolution);           //Calculate printing width

    tofSensor.startRanging();

    started = true;
  }
  else
  {
    logger.info( F( "TOF sensor failed to initialize" ) );
  }  

  vTaskDelete(NULL); // Delete this task when done
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
      closeReadingsCount = 0;

      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for ( int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth )
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          int distance = measurementData.distance_mm[x + y];

          Serial.print( F( "\t" ) );
          Serial.print( distance );

          // Check if the detected distance is within the 1-2 inch range
          if ( ( distance > detectionThresholdLow ) && ( distance < detectionThresholdHigh ) ) 
          {
            closeReadingsCount++;
          }
        }
        Serial.println();
      }
      Serial.println();
    }

    if (closeReadingsCount > majorityThreshold)
    {
      cancelDetected = true;
      cancelGestureTimeout = millis();
      nextCanceltime = millis();
    }

  }
}

bool TOF::cancelGestureDetected()
{
  if ( cancelDetected )
  {
    cancelDetected = false;
    nextCancelflag = true;
    return true;
  }
  else
  {
    return false;
  }  
}

/*
  Hold your palm over the sensor to indicate a Cancel gesture
*/

void TOF::checkForCancelGesture() 
{
  if ( ! started ) return;

  if ( nextCancelflag )
  {
    if ( millis() - nextCanceltime < 5000 )
    {
      cancelDetected = false;
      return;
    }
    nextCancelflag = false;
  }

  cancelDetected = false;

  if ( tofSensor.isDataReady() == true )
  {
    tofSensor.getRangingData(&measurementData);

    closeReadingsCount = 0;

    for ( int i = 0; i < 64; i++ ) 
    {
      int distance = measurementData.distance_mm[ i ];

      // Check if the detected distance is within the 1-2 inch range
      if ( ( distance > detectionThresholdLow ) && ( distance < detectionThresholdHigh ) ) 
      {
        closeReadingsCount++;
      }
    }

    // If the number of close readings exceeds the majority threshold, register a cancel gesture
    if (closeReadingsCount > majorityThreshold)
    {
      cancelDetected = true;
      cancelGestureTimeout = millis();
      nextCanceltime = millis();
    }
  }
}

int TOF::getReadingsCount()
{
  return closeReadingsCount;
}

int TOF::getNextGesture()
{
  if ( cancelDetected )
  {
    cancelDetected = false;
    return cancelled;
  }

  return 0;
}

/* Clears older gestures, allowing new ones to register */

void TOF::removeExpiredGestures()
{
  unsigned long timely = millis();

  if ( timely - cancelGestureTimeout > cancelDuration )
  {
    cancelDetected = false;
  }

  // Add additional timeouts here

}

void TOF::loop()
{
  if ( ! started ) return;

  //removeExpiredGestures();

  if ( millis() - lastPollTime > 2000 ) 
  {
    lastPollTime = millis();
    checkForCancelGesture();
  }

}
