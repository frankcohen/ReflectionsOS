/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

*/

#include "Experience_EasterEggTerri.h"

void Experience_EasterEggTerri::init()
{
  vidflag = true;  
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_EasterEggTerri::setup() 
{
  setExperienceName( EasterEggTerriName );

  Serial.print( EasterEggTerriName );
  Serial.println( F("SETUP") );
  
  video.startVideo( EasterEggTerri_video );
  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_EasterEggTerri::run() 
{
  setRunComplete(true);  // Signal run complete
}

void Experience_EasterEggTerri::teardown() 
{
  if ( video.getStatus() == 0 )
  {
    Serial.println( F("EasterEggTerri TEARDOWN") );
    setTeardownComplete( true );  // Signal teardown complete
  }
}