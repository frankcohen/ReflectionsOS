/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_Pounce.h"

void Experience_Pounce::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_Pounce::setup() 
{
  setExperienceName( PounceName );
  video.startVideo( Pounce_video );
  haptic.playEffect( 76 ); // 100% to 0% slope
  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_Pounce::run() 
{
  setRunComplete(true);  // Signal run complete
}

void Experience_Pounce::teardown() 
{
  if ( video.getStatus() == 0 )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}