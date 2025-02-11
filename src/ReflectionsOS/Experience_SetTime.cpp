/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_SetTime.h"

void Experience_SetTime::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
  timeflag = true;
}

void Experience_SetTime::setup() 
{
  video.startVideo( SetTime_video );

  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_SetTime::run() 
{
  if ( video.getStatus() == 0 )
  {
    setRunComplete(true);  // Signal run complete
  }

}

void Experience_SetTime::teardown() 
{
  setTeardownComplete( true );  // Signal teardown complete
}