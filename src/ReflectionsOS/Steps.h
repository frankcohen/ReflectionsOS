/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _STEPS_
#define _STEPS_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

#include "AccelSensor.h"

extern AccelSensor accel;

#define accelThreshold 11

class Steps
{
  public:
    Steps();
    void begin();
    void loop();
    int howManySteps();
    void resetStepCount();

  private:
    int stepCount;    
    unsigned long lastStepTime;
};

#endif // _STEPS_
