/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_Parallax.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TimeService timeservice;

void Experience_Parallax::init()
{
  vidflag = true;  
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_Parallax::setup() 
{
  Serial.println( "Parallax SETUP" );
  timeservice.setDialActivated( false );
  timeservice.setTimeAnimationActivated( false );
  video.startVideo( ParallaxCat_video );
  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_Parallax::run() 
{
  if ( video.getStatus() == 0 )
  {
    setRunComplete(true);  // Signal run complete
  }
}

void Experience_Parallax::teardown() 
{
  Serial.println( "Parallax TEARDOWN" );
  setTeardownComplete( true );  // Signal teardown complete
}