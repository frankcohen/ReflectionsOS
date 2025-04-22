/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience finds nearby devices and lets them pounce on others

*/

#include "Experience_CatsPlay.h"

void Experience_CatsPlay::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
  setupVidplayed = false;
} 

/*
  Given the devices exchanged their compass heading and the distance (RSSI) is known
  some fun trigonometry shows a bearing between devices. I really should have been
  paying better attention in Art's Algebra class in high school!
*/

float Experience_CatsPlay::calculateBearing( float headingA, float headingB, float rssi ) 
{
  // RSSI to distance conversion (requires calibration)
  float distance = pow(10, (rssi + 40) / (-20)); // Example formula

  // Assuming both boards are in the same plane for simplicity
  // Calculate relative angle (assume 2D plane)
  float angle = atan2(sin(headingB - headingA), cos(headingB - headingA)) * (180 / PI);

  // Calculate bearing from A to B
  float bearingAB = fmod(angle - headingA + 360, 360);

  return bearingAB;
}

void Experience_CatsPlay::setup() 
{
  if ( ! setupVidplayed )
  {
    setupVidplayed = true;
    video.startVideo( CatsPlayFound_video );
    haptic.playEffect( 12 );  //  12 − Triple Click - 100%
  }

  if ( video.getStatus() == 0 )
  {
    directionTimer = millis();
    overallTimer = millis();
    setSetupComplete(true);  // Signal that setup is complete
  }
}

void Experience_CatsPlay::run() 
{
  // Determine direction

  if ( millis() - directionTimer > 1000 )
  {
    directionTimer = millis();
    if ( video.getStatus() != 0 ) return;

    String vid = CatsPlay1_video;
    String head = compass.decodeHeading( compass.getHeading() );
    if ( head == "E" ) vid = CatsPlay1_video;
    if ( head == "NE" ) vid = CatsPlay2_video;
    if ( head == "N" ) vid = CatsPlay3_video;
    if ( head == "NW" ) vid = CatsPlay4_video;
    if ( head == "W" ) vid = CatsPlay5_video;
    if ( head == "SW" ) vid = CatsPlay6_video;
    if ( head == "S" ) vid = CatsPlay7_video;
    if ( head == "SE" ) vid = CatsPlay8_video;
    video.startVideo( vid );

    Serial.print( F("CatsPlay run: ") );
    Serial.println( head );
  }

  if ( accel.tapped() )
  {
    Serial.println( "CatsPlay start pounce to remote devices" );
    blesupport.setPounce( true );    
  }

  if ( ! video.getStatus() ) return;

  // Play time done?

  if ( millis() - overallTimer > 30000 )
  {
    setRunComplete(true);  // Signal run complete

    video.startVideo( Getting_Sleepy_video );
  }
}

void Experience_CatsPlay::teardown() 
{
  if ( video.getStatus() ) return;

  Serial.print( catsplayname );
  Serial.println( F("TEARDOWN") );
  setTeardownComplete( true );  // Signal teardown complete
}