/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

*/

#include "Experience_EasterEggFrank.h"

void Experience_EasterEggFrank::init()
{
  vidflag = true;  
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_EasterEggFrank::setup() 
{
  setExperienceName( EasterEggFrankName );

  Serial.print( EasterEggFrankName );
  Serial.println( F("SETUP") );
  
  video.startVideo( EasterEggFrank_video );
  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_EasterEggFrank::run() 
{
  setRunComplete(true);  // Signal run complete
}

void Experience_EasterEggFrank::teardown() 
{
  if ( video.getStatus() == 0 )
  {
    Serial.println( F("EasterEggFrank TEARDOWN") );
    setTeardownComplete( true );  // Signal teardown complete
  }
}