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
  setExperienceName( eyesname );
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
  if ( millis() - eyestime > ( 15000 + ( dur * 250 ) ) ) 
  {
    setRunComplete(true);  // Signal run complete
    video.setPaused( false );
    return;
  }

  int col = tof.getFingerPos();

  if ( col < 0 || col > 7 ) 
  {
    Serial.print( "Experience_eyes col is out of bounds " );
    Serial.println( col );
    return;
  }

  // Only repaint if finger moved
  if ( col != prevFingerPosCol ) 
  {
    // Erase previous pupils
    if ( prevLeftPupilX >= 0)
    {
      gfx->fillCircle(prevLeftPupilX, 100, 18, COLOR_EYES_LEFT);
      gfx->fillCircle(prevRightPupilX, 100, 18, COLOR_EYES_RIGHT);
      prevFingerPosCol = col;
    }      

    // Map column 0–7 → pixel position for each eye
    int leftPupilX  = map(col, 0, 7, 58, 88);
    int rightPupilX = map(col, 0, 7, 160, 188);

    prevLeftPupilX = leftPupilX;
    prevRightPupilX = rightPupilX;

    // Draw new pupils
    gfx->fillCircle( leftPupilX, 100, 14, COLOR_PUPILS);
    gfx->fillCircle( rightPupilX, 100, 14, COLOR_PUPILS);
  }
}

void Experience_Eyes::teardown() 
{
  if ( tearflag )
  {
    tearflag = false;
    video.setPaused(false);
  }

  if ( ! video.getStatus() ) 
  {
    setTeardownComplete( true );  // Signal teardown complete
    Serial.println( F("Eyes TEARDOWN") );
  }
}