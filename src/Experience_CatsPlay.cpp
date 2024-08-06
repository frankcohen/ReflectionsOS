/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience finds nearby devices and lets them pounce on each other

Next up I am incorporating the ble code into the ReflectionsOfFrank project. There will be two classes: BLE.cpp and BLEMessaging.cpp
BLEMessaging will provide the message level tasks
Inveigle will start a CatsPlay experience based on RSSI distance values from BLE.cpp
CatsPlay experience is going to be big. SetUp will determine the heading to the other cat and play 1 of 8 videos where the cat looks to the other cat. Run will let the user make a TOF gesture to send a Pounce method to the other cat, and making no gestures after 10 seconds runs TearDown to show the cat video ending the play
Inveigle will also call BLEmessaging to see if it received a Pounce from another cat

*/

#include "Experience_CatsPlay.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern BLEServerClass bleServer;
extern BLEClientClass bleClient;
extern TOF tof;
extern Compass compass;

void Experience_CatsPlay::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;

  setupVidplayed = false;

} 

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

  if ( millis() - directionTimer > 2000 )
  {
    directionTimer = millis();
    
    float headingA = compass.getHeading();
    float headingB =   bleServer.getLatestHeading();
    
    // Placeholder RSSI value (should be obtained via BLE communication)
    float rssi = bleClient.getDistance();

    // Calculate bearing from A to B
    float bearingAB = calculateBearing(headingA, headingB, rssi);

    Serial.print( "Bearing from A to B: " );
    Serial.println( bearingAB );

    // Show video of cat's face pointing to other cat

    if ( ( bearingAB > 0 ) && ( bearingAB < 360 ) )
    {
      switch ( int ( ( bearingAB / ( 360 / 8 ) ) + 1 ) )
      {
        case 1:
          video.startVideo( CatsPlay1_video );
          break;
        case 2:
          video.startVideo( CatsPlay1_video );
          break;
        case 3:
          video.startVideo( CatsPlay1_video );
          break;
        case 4:
          video.startVideo( CatsPlay1_video );
          break;
        case 5:
          video.startVideo( CatsPlay1_video );
          break;
        case 6:
          video.startVideo( CatsPlay1_video );
          break;
        case 7:
          video.startVideo( CatsPlay1_video );
          break;
        case 8:
          video.startVideo( CatsPlay1_video );
          break;
      }
    }
  }

  // Pounce gesture made? Send pounce message

  if ( bleServer.isPounced() )
  {
    Serial.println( "CatsPlay pounced" );

    // Pounce gesture message received?
    video.startVideo( Pounce_video );
  }

  // Play time done?

  if ( millis() - overallTimer > 30000 )
  {
    setRunComplete(true);  // Signal run complete
  }

  // User gestured a pounce to the connected cat

  int mygs = tof.getGesture();
  if ( mygs == TOF::None ) return;

  Serial.print( "CatsPlay detects TOF gesture " );
  Serial.print( mygs );
  Serial.println( " sending pounce");

  bleClient.sendPounce();
}

void Experience_CatsPlay::teardown() 
{
  if ( video.getStatus() == 0 )
  {
    video.startVideo( Sleep_video );
    setTeardownComplete( true );  // Signal teardown complete
  }
}