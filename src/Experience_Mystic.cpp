/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_Mystic.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
Mystic mystic;

void Experience_Mystic::init()
{
  vidflag = true;  
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
} 

void Experience_Mystic::setup() 
{
  if ( vidflag )
  {
    video.startVideo( MysticCat_video );
    timeflag = true;
    vidflag = false;
    setSetupComplete(true);  // Signal that setup is complete
  }
}

void Experience_Mystic::run() 
{
  mystic.runShowTellAnswers();
  tearflag = true;
  setRunComplete(true);  // Signal run complete
}

void Experience_Mystic::teardown() {
  // Teardown code for Experience_Awake

  if ( video.getStatus() == 0 )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}