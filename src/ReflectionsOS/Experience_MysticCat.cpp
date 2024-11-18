/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_MysticCat.h"

void Experience_MysticCat::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_MysticCat::setup() 
{
  video.startVideo( MysticCat_video );
  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_MysticCat::run() 
{
  setRunComplete(true);  // Signal run complete
}

void Experience_MysticCat::teardown() {

  if ( video.getStatus() == 0 )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}