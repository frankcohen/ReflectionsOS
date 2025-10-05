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
#include <math.h>

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

float Experience_Eyes::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int Experience_Eyes::eyesComputeAccelCol() const
{
    // Same mapping as Experience_Parallax:
    // NOTE: ensure 'accel' is properly declared/visible via its header.
    float xraw = accel.getXreading();
    int x = constrain( (int)round( mapFloat(xraw, -6, 6, 1.0f, 9.0f) ), 1, 9 );
    return x;
}

int Experience_Eyes::eyesFuseColumns(int tofCol9)
{
    // Clamp the incoming TOF col just in case
    int tcol = constrain(tofCol9, 1, 9);

    // Read accel-based column
    int acol = eyesComputeAccelCol(); // already 1..9

    // Simple 50/50 average
    float blended = 0.5f * (float)tcol + 0.5f * (float)acol;

    // EMA smoothing on the blended value
    if (!_eyes_hasEma) {
        _eyes_ema = blended;
        _eyes_hasEma = true;
    } else {
        _eyes_ema = (EYES_EMA_ALPHA * blended) + ((1.0f - EYES_EMA_ALPHA) * _eyes_ema);
    }

    // Quantize to [1..9]
    _eyes_finalCol = constrain((int)lroundf(_eyes_ema), 1, 9);
    return _eyes_finalCol;
}

void Experience_Eyes::setup() 
{
  if (vidflag)
  {
    setExperienceName( eyesname );
    Serial.print( eyesname );
    Serial.println( F("SETUP") );

    video.startVideo( EyesFollowFinger_video );

    vidflag = false;
    tearflag = true;              // <- uses the base class member
    pace = millis();
  }

  if ( video.getVideoTime() > 2800 )
  {
    eyestime = millis();
    dur = random( 1, 5 );
    video.setPaused( true );
    
    prevFingerPosCol = -1;  // store FINAL column here
    prevFingerDist = -1;

    prevLeftPupilX = -1;
    prevRightPupilX = -1;
        
    setSetupComplete( true );
  }
}

void Experience_Eyes::run() 
{
  if ( millis() - eyestime > ( 15000 + ( dur * 250 ) ) ) 
  {
    setRunComplete(true);
    video.setPaused( false );
    return;
  }

  if ( millis() - pace < 500 ) return;
  pace = millis();

  // TOF returns 0..7 (or possibly other). Map to 1..9 and clamp.
  int col = tof.getFingerPos();
  int tofCol9 = constrain(map(col, 0, 7, 1, 9), 1, 9);

  // Average TOF with tilt (plus EMA smoothing)
  int finalCol = eyesFuseColumns(tofCol9);

  Serial.print( "Eyes: " );
  Serial.print( col );
  Serial.print( ", " );
  Serial.print( tofCol9 );
  Serial.print( ", " );
  Serial.println( finalCol );

  // Only repaint if FINAL column moved
  if (finalCol != prevFingerPosCol) 
  {
    // Erase previous pupils
    if ( prevLeftPupilX != -1 )
    {
      gfx->fillCircle(prevLeftPupilX, 100, 18, COLOR_EYES_LEFT);
      gfx->fillCircle(prevRightPupilX, 100, 18, COLOR_EYES_RIGHT);
    }

    // Map FINAL column [1..9] → pixel positions
    int leftPupilX  = map(finalCol, 1, 9, 58, 88);
    int rightPupilX = map(finalCol, 1, 9, 160, 188);

    // Draw new pupils
    gfx->fillCircle( leftPupilX, 100, 14, COLOR_PUPILS );
    gfx->fillCircle( rightPupilX, 100, 14, COLOR_PUPILS );

    // Save for next frame
    prevLeftPupilX   = leftPupilX;
    prevRightPupilX  = rightPupilX;
    prevFingerPosCol = finalCol;
  }
}

void Experience_Eyes::teardown() 
{
  if ( tearflag )
  {
    tearflag = false;         // base member
    video.setPaused(false);
  }

  if ( ! video.getStatus() ) 
  {
    setTeardownComplete( true );
    Serial.println( F("Eyes TEARDOWN") );
  }
}
