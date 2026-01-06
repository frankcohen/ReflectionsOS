/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_ShowTime.h"

void Experience_ShowTime::init()
{
  vidflag = true;
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_ShowTime::setup() 
{
  if ( vidflag )
  {
    setExperienceName( showtimename );

    Serial.print( showtimename );
    Serial.println( F("SETUP") );

    video.startVideo( ShowTime_video );
    timeflag = true;
    vidflag = false;
    tearflag = true;    
  }

  if ( video.getVideoTime() > 3600 )
  {
    video.setPaused( true );
    setSetupComplete(true);  // Signal that setup is complete
  }
}

void Experience_ShowTime::run() 
{
  if ( timeflag )
  {
    int sIndex = random(0, 34);
    textmessageservice.startShow( TextMessageExperiences::ShowDigitalTimeFunMessages, timefunmessages[ sIndex ][ 0 ], timefunmessages[ sIndex ][ 1 ] );  
    timeflag = false;
  }
  else
  {
    if ( ! textmessageservice.active() )
    {
      video.setPaused( false );
      tearflag = true;
      setRunComplete(true);  // Signal run complete
    }
  }
}

void Experience_ShowTime::teardown() {
    // Teardown code for Experience_ShowTime

  if ( tearflag )
  {
    tearflag = false;
    Serial.print( showtimename );
    Serial.println( F("TEARDOWN") );
  }

  if ( ! video.getStatus() )
  {
    timeflag = true;
    setTeardownComplete( true );  // Signal teardown complete
  }

}