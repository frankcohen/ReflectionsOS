/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires PNGdec library https://github.com/bitbank2/PNGdec

*/

#include "WatchFaceExperiences.h"

WatchFaceExperiences::WatchFaceExperiences() : currentState(MAIN) {}

void WatchFaceExperiences::begin() 
{
  watchFaceMain.begin();
}

void WatchFaceExperiences::loop() 
{

  // Sense for display off mode




  switch (currentState) 
  {
    case MAIN:
      watchFaceMain.loop();

      // Sense gestures

      // Change mode Gesture








      break;
    case DISPLAYING_DIGITAL_TIME:
      // Placeholder for displaying digital time state
      //watchFaceMain.loop();
      break;
    case SETTING_DIGITAL_TIME:
      // Placeholder for setting digital time state
      break;
    case DISPLAYING_HEALTH_STATISTICS:
      // Placeholder for displaying health statistics state
      break;
    case SETTING_HEALTH_STATISTICS:
      // Placeholder for setting health statistics state
      break;
    case DISPLAYING_TIMER:
      // Placeholder for displaying timer state
      break;
    case SETTING_TIMER:
      // Placeholder for setting timer state
      break;

    case DISPLAY_OFF:
      // Placeholder for setting timer state
      break;
  }

  // Time-out of modes

  /*

  if ( currentState == MAIN ) return;

  if ( currentState == DISPLAYING_DIGITAL_TIME || 
    currentState == SETTING_DIGITAL_TIME || 
    currentState == DISPLAYING_HEALTH_STATISTICS || 
    currentState == SETTING_HEALTH_STATISTICS || 
    currentState == DISPLAYING_TIMER || 
    currentState == SETTING_TIMER ) )
  {
    currentState == MAIN;
  }
    

  */

}

void WatchFaceExperiences::setState(WatchFaceState newState) {
    currentState = newState;
}
