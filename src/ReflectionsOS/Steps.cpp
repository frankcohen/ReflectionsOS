/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Step counter using accelerometer

*/

#include "Steps.h"

Steps::Steps(){}

void Steps::begin()
{ 
  lastStepTime = millis();
}

void Steps::loop()
{
  uint32_t currentTime = millis();

  if ( currentTime - lastStepTime > 500 ) 
  {
    float x = accel.getXreading();
    float y = accel.getYreading();
    float z = accel.getZreading();

    // Calculate magnitude of acceleration
    float magnitude = sqrt(x * x + y * y + z * z);

    // Detect steps based on threshold and timing
    if (magnitude > accelThreshold) 
    {
      lastStepTime = currentTime;

      stepCount++;
    }
  }
}

int Steps::howManySteps() {
    return stepCount;
}

void Steps::resetStepCount() {
    stepCount = 0;
}

