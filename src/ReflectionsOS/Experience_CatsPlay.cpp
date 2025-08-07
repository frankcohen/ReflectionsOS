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
  setExperienceName( catsplayname );

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

    // Get the local heading (from the compass)
    float localHeading = compass.getHeading();

    // Get the remote heading and RSSI from BLEsupport
    float remoteHeading = blesupport.getHeading();
    int rssi = blesupport.getRSSI();

    // Use the calculateBearing function to find the relative bearing
    float bearing = calculateBearing(localHeading, remoteHeading, rssi);

    // Determine which video to play based on the bearing
    String vid = CatsPlay1_video;  // Default video

    if (bearing >= 0 && bearing < 45) {
      vid = CatsPlay1_video;  // East (E)
    } else if (bearing >= 45 && bearing < 90) {
      vid = CatsPlay2_video;  // Northeast (NE)
    } else if (bearing >= 90 && bearing < 135) {
      vid = CatsPlay3_video;  // North (N)
    } else if (bearing >= 135 && bearing < 180) {
      vid = CatsPlay4_video;  // Northwest (NW)
    } else if (bearing >= 180 && bearing < 225) {
      vid = CatsPlay5_video;  // West (W)
    } else if (bearing >= 225 && bearing < 270) {
      vid = CatsPlay6_video;  // Southwest (SW)
    } else if (bearing >= 270 && bearing < 315) {
      vid = CatsPlay7_video;  // South (S)
    } else if (bearing >= 315 && bearing < 360) {
      vid = CatsPlay8_video;  // Southeast (SE)
    }

    // Start the selected video
    video.startVideo(vid);

    // Print the calculated bearing for debugging
    Serial.print(F("CatsPlay run: Calculated Bearing: "));
    Serial.println(bearing);
    Serial.print(F("Local Heading: "));
    Serial.println(localHeading);
    Serial.print(F("Remote Heading: "));
    Serial.println(remoteHeading);
    Serial.print(F("RSSI: "));
    Serial.println(rssi);
    Serial.print(F("Selected Video: "));
    Serial.println(vid);
  }

  if ( accel.getSingleTap() )
  {
    Serial.println( "CatsPlay send pounce to other cats" );
    blesupport.sendPounce();    
  }

  // Play time done?

  if ( millis() - overallTimer > 30000 )
  {
    setRunComplete(true);  // Signal run complete

    // video.startVideo( Getting_Sleepy_video );
  }
}

void Experience_CatsPlay::teardown() 
{
  if ( video.getStatus() ) return;

  Serial.print( catsplayname );
  Serial.println( F("TEARDOWN") );
  setTeardownComplete( true );  // Signal teardown complete
}