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

  taskName1 = F("");
  taskName2 = F("");
  taskName3 = F("");
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

void SystemLoad::printHeapSpace( String message )
{
 // Get total free heap available in bytes.
  uint32_t freeHeap = heap_caps_get_free_size( MALLOC_CAP_8BIT );
  // Get the size of the largest contiguous free block.
  uint32_t largestBlock = heap_caps_get_largest_free_block( MALLOC_CAP_8BIT );
  // Compute a rough fragmentation ratio.
  float fragRatio = (float)freeHeap / largestBlock;

  /*
  Serial.println( "heap_caps_print_heap_info:" );
  heap_caps_print_heap_info(MALLOC_CAP_8BIT);
  */
  
  Serial.print( message );
  Serial.print(F(", Heap: "));
  Serial.print( freeHeap );
  Serial.print(F(" , FreeBlock: "));
  Serial.print( largestBlock );
  Serial.print(F(" , Fragmentation ratio: "));
  Serial.println( fragRatio );

  /*
  size_t freeHeapDefault = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  Serial.print(F("Default Free Heap: "));
  Serial.print(freeHeapDefault);
  Serial.println(F(" bytes"));
  */
}

void SystemLoad::loop() {
  unsigned long currentMillis = millis();

  // At the end of the interval, calculate averages and print them
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;

    Serial.print(F("Load: "));
    Serial.print( float( accumulatedTaskTime ) / float( interval) );
    Serial.print(F("% "));
    Serial.print( float( accumulatedTaskTime1 ) / float( interval) );
    Serial.print(F("% "));
    Serial.print( float( accumulatedTaskTime2 ) / float( interval) );
    Serial.print(F("% "));
    Serial.print( float( accumulatedTaskTime3 ) / float( interval) );
    Serial.print(F("% "));
    printHeapSpace( ", memory" );

    // Reset the accumulated values for the next interval
    accumulatedTaskTime = 0;
  }
}

void SystemLoad::printStats() 
{
   Serial.print(F("Task: "));
    Serial.print( accumulatedTaskTime );
    Serial.print(F(", Interval: "));
    Serial.println( interval );
}
