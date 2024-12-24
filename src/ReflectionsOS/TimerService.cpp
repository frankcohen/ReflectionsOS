/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Step counter using accelerometer

*/

#include "TimerService.h"

TimerService::TimerService(){}

void TimerService::begin()
{ 
  timertimer = millis();
  alarmduration = millis();
  running = false;
  timeleft = 20;    // Minutes
  alarmnotice = false;
}

void TimerService::setTime( int minutes ) 
{
  timeleft = minutes;
}

int TimerService::getTime()
{
  return timeleft;
}

void TimerService::start()
{
  running = true;
  alarmnotice = false;
}

void TimerService::stop()
{
  running = false;
  alarmnotice = false;
}

// Returns true if TimerService is in alarm

bool TimerService::status()
{
  return alarmnotice;
}

void TimerService::loop()
{
  if ( running )
  {
    if ( millis() - timertimer > ( 60 * 1000 ) )
    {
      timertimer = millis();

      if ( timeleft > 0 )
      {
        timeleft = timeleft - 1;
        Serial.print( "Timer " );
        Serial.println( timeleft );
      }
      else
      {
        Serial.println( "Timer done" );
        alarmnotice = true;
        alarmduration = millis();
        running = false;
      }
    }
  }

  // Alarm notice shows for up to 1 minute

  if ( alarmnotice )
  {
    if ( millis() - alarmduration > ( 60 * 1000 ) )
    {
      alarmnotice = false;
      Serial.println( "Timer alarm duration done" );
    }
  }
  
}