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
    dur = random( 1, 5 );

    paralaxDuration = millis();
    parallaxWaitTime = millis();

    video.setPaused( true );

    setSetupComplete( true );  // Signal that setup is complete
  }
}

void Experience_Parallax::run() {
  // 1) still in initial pause period?
  if (millis() - paralaxDuration < (20000 + dur * 250)) {
    if (millis() - parallaxWaitTime < 500) return;
    parallaxWaitTime = millis();

    // 2) read & clamp accel to g
    float x = accel.getXreading() * 0.004f;     // raw → g
    float y = accel.getYreading() * 0.004f;
    x = constrain(x, -8.0f, 8.0f);
    y = constrain(y, -8.0f, 8.0f);

    // 3) compute and smooth deltas
    float dx = x - prevX, dy = y - prevY;
    prevX = x;  prevY = y;
    motionX = motionAlpha * dx + (1 - motionAlpha) * motionX;
    motionY = motionAlpha * dy + (1 - motionAlpha) * motionY;

    // 4) detect motion above threshold
    const float threshold = 0.01f;
    bool hasMotion = (fabs(motionX) > threshold || fabs(motionY) > threshold);
    if (hasMotion) lastMotionTime = millis();

    // 5) decide single-step direction
    int deltaCol = 0, deltaRow = 0;
    if (hasMotion) {
      // pick dominant axis only
      if (fabs(motionX) > fabs(motionY)) {
        if (motionX < -threshold) deltaCol = -1;
        else if (motionX > threshold) deltaCol = +1;
      } else {
        if (motionY < -threshold) deltaRow = -1;
        else if (motionY > threshold) deltaRow = +1;
      }
    } else {
      // 6) after 3 s of no motion, start decaying to center
      if (millis() - lastMotionTime >= 3000) {
        int r = (currentFrame - 1)/3 + 1;
        int c = (currentFrame - 1)%3 + 1;
        if (r < 2)      deltaRow = +1;
        else if (r > 2) deltaRow = -1;
        if (c < 2)      deltaCol = +1;
        else if (c > 2) deltaCol = -1;
      }
    }

    // 7) apply one-step move within [1..3]×[1..3]
    int curRow = (currentFrame - 1)/3 + 1;
    int curCol = (currentFrame - 1)%3 + 1;
    int newRow = constrain(curRow + deltaRow, 1, 3);
    int newCol = constrain(curCol + deltaCol, 1, 3);

    int targetFrame = (newRow - 1)*3 + newCol;  // 1..9
    targetFrame = constrain(targetFrame, 1, 9);

    // 8) update & display
    if (targetFrame != currentFrame) {
      currentFrame = targetFrame;
      String mef = "cat" + String(currentFrame) + "_parallax_baseline.jpg";
      watchfacemain.drawImageFromFile(mef, true, 0, 0);
      watchfacemain.show();
      Serial.printf(
        "motionX:%.3f motionY:%.3f → frame %d\n",
        motionX, motionY, currentFrame
      );
    }
  }
  else {
    // after the main period, resume video
    video.setPaused(false);
    if (video.getStatus() == 0) {
      setRunComplete(true);
    }
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

  if ( video.getStatus() == 0 )
  {
    setTeardownComplete( true );  // Signal teardown complete
  }
}

