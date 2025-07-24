/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_Shaken.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;

void Experience_Shaken::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_Shaken::setup() 
{
  setExperienceName( shakenname );

  Serial.print( shakenname );
  Serial.println( F("SETUP") );

  video.startVideo( Shaken_video );
  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_Shaken::run() 
{
  setRunComplete(true);  // Signal run complete
}

void Experience_Shaken::teardown() 
{
  if ( video.getStatus() == 0 )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}