/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/
#include "SystemLoad.h"

SystemLoad::SystemLoad() {} 

void SystemLoad::begin()
{
  interval = 15 * 1000;
  previousMillis = 0;

  // Initialize accumulated values
  accumulatedTaskTime = 0;

  taskName1 = "";
  taskName2 = "";
  taskName3 = "";
  accumulatedTaskTime1 = 0;
  accumulatedTaskTime2 = 0;
  accumulatedTaskTime3 = 0;

  oldTime = millis();
}

void SystemLoad::logtasktime( unsigned long tsktime, int mes,  String mname )
{
  if ( mes == 0 ) accumulatedTaskTime = accumulatedTaskTime + tsktime;
  if ( mes == 1 ) accumulatedTaskTime1 = accumulatedTaskTime1 + tsktime;
  if ( mes == 2 ) accumulatedTaskTime2 = accumulatedTaskTime2 + tsktime;
  if ( mes == 3 ) accumulatedTaskTime3 = accumulatedTaskTime3 + tsktime;
}

void SystemLoad::loop() {
  unsigned long currentMillis = millis();

  // At the end of the interval, calculate averages and print them
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;

    Serial.print("Load: ");
    Serial.print( float( accumulatedTaskTime ) / float( interval) );
    Serial.print("% ");
    Serial.print( float( accumulatedTaskTime1 ) / float( interval) );
    Serial.print("% ");
    Serial.print( float( accumulatedTaskTime2 ) / float( interval) );
    Serial.print("% ");
    Serial.print( float( accumulatedTaskTime3 ) / float( interval) );
    Serial.println("% ");

    // Reset the accumulated values for the next interval
    accumulatedTaskTime = 0;
  }
}

void SystemLoad::printStats() 
{
   Serial.print("Task: ");
    Serial.print( accumulatedTaskTime );
    Serial.print(", Interval: ");
    Serial.println( interval );
}
