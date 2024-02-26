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
  pinMode( TOFPower, OUTPUT);    // Power control for TOF sensor
  digitalWrite( TOFPower, LOW);

  started = false;

  Serial.println( F( "Initializing TOF sensor" ) );
  if ( myImager.begin( 0x29, Wire ) == false )
  {
    Serial.println( F("TOF sensor not found") );
    return;
  }

  myImager.setResolution(8*8); //Enable all 64 pads
  
  imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  myImager.startRanging();

  started = true;
}

boolean TOF::test()
{
  return started;  
}

void TOF::printTOF()
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

void TOF::loop()
{
}
