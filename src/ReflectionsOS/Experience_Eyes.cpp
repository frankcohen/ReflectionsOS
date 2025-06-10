/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_Eyes.h"

void Experience_Eyes::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  vidflag = true;  
  stopped = false;
  idle = false;
  
  eyeposx = 50;   // For debugging, ok to delete later
  eyeposy = 100;
  eyedist = 14;
} 

void Experience_Eyes::setup() 
{
  if ( vidflag )
  {
    Serial.print( eyesname );
    Serial.println( F("SETUP") );

    video.startVideo( EyesFollowFinger_video );

    vidflag = false;
    tearflag = true;
  }

  if ( video.getVideoTime() > 2800 )
  {
    //gfx -> fillCircle( 68, 100, 14, COLOR_PUPILS );   // Left eye
    //gfx -> fillCircle( 172, 100, 14, COLOR_PUPILS );   // Right eye
    //prevLeftPupilX = 68;
    //prevRightPupilX = 172;

    eyestime = millis();
    dur = random( 1, 5 );
    video.setPaused( true );
    
    prevFingerPosCol = -1;
    prevFingerDist = -1;

    prevLeftPupilX = -1;
    prevRightPupilX = -1;
        
    setSetupComplete( true );  // Signal that setup is complete
  }
}

void Experience_Eyes::run() 
{
  if (millis() - eyestime < (20000 + dur * 250)) 
  {
    int col = constrain(tof.getFingerPos(), 0, 5);
    float dist = tof.getFingerDist();

    if (col < 0 || dist <= 4 || dist > 130) return;

    // Only repaint if finger moved
    if (col != prevFingerPosCol || fabs(dist - prevFingerDist) > 1.0f) 
    {
      // Erase previous pupils
      if (prevLeftPupilX >= 0)
      {
        gfx->fillCircle(prevLeftPupilX, 100, 18, COLOR_EYES_LEFT);
        gfx->fillCircle(prevRightPupilX, 100, 18, COLOR_EYES_RIGHT);
        prevFingerPosCol = col;
        prevFingerDist = dist;
      }      

      // Map column 0–5 → pixel position for each eye
      int leftMapped  = map(col, 0, 5, 58, 88);
      int rightMapped = map(col, 0, 5, 160, 188);

      // Clamp dist and map to alpha range
      dist = constrain(dist, 20.0f, 100.0f);
      float alpha = (100.0f - dist) / 80.0f;

      int leftTarget  = 88;  // Full convergence position (left eye inward)
      int rightTarget = 160; // Full convergence position (right eye inward)

      int leftPupilX  = leftMapped  + alpha * (leftTarget  - leftMapped);
      int rightPupilX = rightMapped + alpha * (rightTarget - rightMapped);

      prevLeftPupilX = leftPupilX;
      prevRightPupilX = rightPupilX;

      // Draw new pupils
      gfx->fillCircle( leftPupilX, 100, 14, COLOR_PUPILS);
      gfx->fillCircle( rightPupilX, 100, 14, COLOR_PUPILS);
    }
  } 
  else 
  {
    video.setPaused(false);
    if (video.getStatus() == 0) {
      setRunComplete(true);  // Signal run complete
    }
  }
}


void Experience_Eyes::teardown() 
{
  Serial.println( F("Eyes TEARDOWN") );
  setTeardownComplete( true );  // Signal teardown complete
}