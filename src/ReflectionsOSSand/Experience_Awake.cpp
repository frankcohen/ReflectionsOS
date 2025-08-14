/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Shows wake-up video

*/

#include "Experience_Awake.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;

void Experience_Awake::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
}

void Experience_Awake::setup() 
{
  setExperienceName( awakename );

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  if (reason == ESP_SLEEP_WAKEUP_EXT1) 
  {   
    video.startVideo( WatchFaceOpener_video );
  } 
  else 
  {
    video.startVideo( OutOfTheBox_video );
  }

  video.setPaused( true );
  setSetupComplete( true );  // Signal that setup is complete
}

void Experience_Awake::run() 
{
  setRunComplete(true);  // Signal run complete
}

void Experience_Awake::teardown() {
  // Teardown code for Experience_Awake
  if ( ! video.getStatus() )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}