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

  //video.startVideo( ShowTime_video );
  //setSetupComplete(true);  // Signal that setup is complete

  if ( vidflag )
  {
    video.startVideo( ShowTime_video );
    timeflag = true;
    vidflag = false;
  }

  if ( video.getVideoTime() > 3500 )
  {
    video.setPaused( true );
    setSetupComplete(true);  // Signal that setup is complete
  }
}

void Experience_ShowTime::run() 
{
  if ( timeflag )
  {
    textmessageservice.startShow( TextMessageExperiences::DigitalTime, "", "" );  
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

    //video.setPaused( false );

    if ( video.getStatus() == 0 )
    {
      setTeardownComplete( true );  // Signal teardown complete
    }
}