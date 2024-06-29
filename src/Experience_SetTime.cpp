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

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TimeService timeservice;

void Experience_SetTime::setup() 
{
  if ( video.getStatus() == 0 )
  {
    video.startVideo( Settime_video );
    Serial.print( "Experience_SetTime setup, start video " );
    Serial.println( Settime_video );
  }

  if ( video.getVideoTime() > 2500 )
  {
    Serial.println( "SetTime setup complete" );
    video.setPaused( true );
    timeflag = true;
    setSetupComplete(true);  // Signal that setup is complete
  }

  if ( video.getStatus() == 0 )
  { 
    Serial.println( "SetTime setup moved to stopped" );
    setStopped( true );
    return;
  }

}

// Run code for Experience_SetTime

void Experience_SetTime::run() 
{
  if ( timeflag )
  {
    timeservice.startShow( 0 );     // Show saying plus hour and minute
    timeflag = false;
  }
  else
  {
    if ( ! timeservice.getActivated() )
    {
      video.setPaused( false );
      tearflag = true;
      setRunComplete(true);  // Signal that run is complete
    }
  }
}

void Experience_SetTime::teardown() {
    // Teardown code for Experience_SetTime

    if ( video.getStatus() == 0 )
    {
      setTeardownComplete ( true );  // Signal that run is complete
    }
}