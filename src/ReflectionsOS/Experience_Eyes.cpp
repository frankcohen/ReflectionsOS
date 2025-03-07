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

  prevFingerPosRow = -1;
  prevFingerPosCol = -1;
  prevFingerDist = -1;

  prevLeftPupilX = -1;
  prevRightPupilX = -1;
  
  eyeposx = 50;   // For debugging, ok to delete later
  eyeposy = 100;
  eyedist = 14;
} 

void Experience_Eyes::setup() 
{
  if ( vidflag )
  {
    Serial.print( eyesname );
    Serial.println( "SETUP" );

    video.startVideo( EyesFollowFinger_video );

    vidflag = false;
    tearflag = true;
  }

  if ( video.getVideoTime() > 3000 )
  {
    gfx -> fillCircle( 68, 100, 14, COLOR_PUPILS );   // Left eye
    gfx -> fillCircle( 172, 100, 14, COLOR_PUPILS );   // Right eye
    prevLeftPupilX = 68;
    prevRightPupilX = 172;

    eyestime = millis();
    dur = random( 1, 5 );
    video.setPaused( true );
    setSetupComplete( true );  // Signal that setup is complete
  }
}

void Experience_Eyes::run() 
{
  if ( millis() - eyestime < ( 20000 + ( dur * 250 ) ) )
  {
    int row = tof.getFingerPosRow();
    int col = tof.getFingerPosCol();
    float dist = tof. getFingerDist();

    if ( ( col != prevFingerPosCol ) && ( dist > 0 ) )
    { 
      prevFingerPosCol = col;

      /*
      String myf = String( col );
      myf += " ";
      myf += dur;
      myf += " ";
      myf += dist;
      Serial.println( myf );
      */
      
      // Erase previous pupils
      if ( prevLeftPupilX != -1 ) gfx -> fillCircle( prevLeftPupilX, 100, 20, COLOR_EYES_LEFT );
      if ( prevRightPupilX != -1 ) gfx -> fillCircle( prevRightPupilX, 100, 20, COLOR_EYES_RIGHT );

      // For the left eye, map from 0–5 to 58–88 pixels.
      int leftMapped = map( col, 0, 5, 58, 88);

      // For the right eye, map from 0–5 to 160–188 pixels.
      int rightMapped = map( col, 0, 5, 160, 188);

      // When distance is 50, alpha = 0 (no convergence).
      // When distance is 5, alpha = 1 (full convergence toward the inner edge).
      float alpha = (50 - dist) / 45.0;  // (50 - 5 = 45)

      int leftConverged  = 88;
      int rightConverged = 160;

      int leftPupil  = leftMapped  + alpha * (leftConverged  - leftMapped);
      int rightPupil = rightMapped + alpha * (rightConverged - rightMapped);

      prevLeftPupilX = leftPupil;
      prevRightPupilX = rightPupil;

      gfx -> fillCircle( leftPupil, 100, 14, COLOR_PUPILS );   // Left eye
      gfx -> fillCircle( rightPupil, 100, 14, COLOR_PUPILS );   // Right eye

    }
  }
  else
  {
    video.setPaused( false );

    if ( video.getStatus() == 0 )
    {
      setRunComplete(true);  // Signal run complete
    }
  }
}

void Experience_Eyes::teardown() 
{
  Serial.println( "Eyes TEARDOWN" );
  setTeardownComplete( true );  // Signal teardown complete
}