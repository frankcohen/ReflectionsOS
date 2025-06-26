/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

3D wallpaper with parallax effect
Move the device, accelerometer senses position, changes wallpaper

Requires 6 jpg images to be named /REFLECTIONS/cat1_parallax_baseline.jpg, 2, 3, etc.
Images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

Uses the X axis of the Reflections board accellerometer to determine the image.
Future expansion should incorporate the Y and Z axis.

*/

#include "Experience_Parallax.h"

void Experience_Parallax::init()
{
  vidflag    = true;
  setupComplete    = false;
  runComplete      = false;
  teardownComplete = false;
  stopped    = false;
  idle       = false;

  parallaxWaitTime   = millis();
  paralaxDuration    = millis();

  // initialize motion state
  prevX = prevY = motionX = motionY = 0.0f;
  currentFrame     = 5;
  lastMotionTime   = millis();
} 

void Experience_Parallax::setup() 
{
  if ( vidflag )
  {
    Serial.println( parallaxname );
    Serial.println( F("SETUP") );

    video.startVideo( ParallaxCat_video );

    vidflag = false;
    tearflag = true;
  }

  if ( video.getVideoTime() > 2600 )
  {
    dur = random( 2, 6 );
    paralaxDuration = millis();
    parallaxWaitTime = millis();
    video.setPaused( true );
    setSetupComplete( true );  // Signal that setup is complete
  }
}

float Experience_Parallax::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Experience_Parallax::run() 
{
  // 1) still in initial pause period?
  if ( millis() - paralaxDuration > ( 30000 + ( dur * 2000 ) ) ) 
  {
    video.setPaused( false );
    setRunComplete(true);
    return;
  }

  if (millis() - parallaxWaitTime < 500) return;
  parallaxWaitTime = millis();

  float xraw = accel.getXreading();

  int x = constrain( round( mapFloat(xraw, -6, 6, 1.0f, 9.0f) ), 1, 9 );

  if ( x != currentFrame ) 
  {
    currentFrame = x;
    String mef = "cat" + String(currentFrame) + "_parallax_baseline.jpg";
    watchfacemain.drawImageFromFile(mef, true, 0, 0);
    watchfacemain.show();
    Serial.printf(
      "motionX:%.3f motionY:%.3f → frame %d\n",
      motionX, motionY, currentFrame
    );
  }
}

void Experience_Parallax::teardown() 
{
  if ( tearflag )
  {
    tearflag = false;
    Serial.print( parallaxname );
    Serial.println( F("TEARDOWN") );
  }

  if ( ! video.getStatus() )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}

