/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Step counter using accelerometer

*/

#include "Timer.h"

Timer::Timer(){}

void Timer::begin()
{ 
  timertimer = millis();
  running = false;
  timeleft = 0;
}

void Timer::setTime( int minutes ) 
{
  timeleft = minutes;
}

int Timer::getTime()
{
  return timeleft;
}

void Timer::start()
{
  running = true;
}

void Timer::stop()
{
  running = false;
}

// Returns true if timer is in alarm

bool Timer::status()
{
  if ( ! running ) return false;
  if ( timeleft > 0 ) return false;
  return true;
}

void Timer::loop()
{
  if ( ! status() )
  {
    if ( millis() - timertimer > ( 60 * 1000 ) )
    {
      timertimer = millis();

      if ( timeleft > 0 ) timeleft--;
    }
  }
}