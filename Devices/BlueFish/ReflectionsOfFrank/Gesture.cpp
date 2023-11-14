/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
*/

#include "Gesture.h"

Gesture::Gesture(){}

void Gesture::begin()
{ 
  pinMode( GesturePower, OUTPUT);    // Power control for gesture sensor
  digitalWrite( GesturePower, LOW);

  started = false;

  Serial.println( F( "Initializing gesture sensor" ) );
  if ( myImager.begin( 0x29, Wire ) == false )
  {
    Serial.println( F("Gesture sensor not found") );
    return;
  }

  myImager.setResolution(8*8); //Enable all 64 pads
  
  imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  myImager.startRanging();

  started = true;
}

boolean Gesture::test()
{
  return started;  
}

void Gesture::printGesture()
{
  if ( ! started ) return;

  //Poll sensor for new data
  if (myImager.isDataReady() == true)
  {
    if (myImager.getRangingData(&measurementData)) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
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

void Gesture::loop()
{
}
