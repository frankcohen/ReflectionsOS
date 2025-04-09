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

WatchFaceExperiences::WatchFaceExperiences() {}

void WatchFaceExperiences::begin() 
{
  watchfacemain.begin();
  currentface = MAIN;
}

/* Returns true when watch face is ok with the processor being put to sleep, to save battery life */

bool WatchFaceExperiences::okToSleep() 
{
  switch ( currentface ) 
  {
    case MAIN:
      return watchfacemain.okToSleep();
      break;
    case MINIMAL:
      break;
    case MOON:
      break;
    case BLAIR:
      break;
  
  }
}

void WatchFaceExperiences::loop() 
{
  switch ( currentface ) 
  {
    case MAIN:
      watchfacemain.loop();
      break;
    case MINIMAL:
      break;
    case MOON:
      break;
    case BLAIR:
      break;
  
  }

}