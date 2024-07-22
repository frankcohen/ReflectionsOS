/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

Inveigle - verb. To persuade (someone) to do something by means of deception or flattery

That describes the nature of an entertainment experience controller. The
Inveigle controls which part of the experience happens and when.

Experiences.cpp is an abstract base class to implement the Inveigle's actions.
Each experience has a startup, run, and teardown method. Inveigle uses gesture,
time, and event sensing to run an experience. It runs the startup and run
methods. It uses gesture and event sensing to interrupt the run method and
skip to the teardown method. 

Connect experiences to the Inveigle by adding an Experience implementation to 
the #include list and class instantiation method below.

*/

#include "Inveigle.h"

// Add an include for each experience here
#include "Experience_Awake.h"
#include "Experience_ShowTime.h"
#include "Experience_Sleep.h"
#include "Experience_SetTime.h"
#include "Experience_Mystic.h"

extern Video video;
extern TimeService timeservice;

// and add the experience here too

Inveigle::Inveigle() : currentExperience(nullptr), currentState(SETUP) 
{
  // Add instances of each experience to the vector

  experiences.push_back( new Experience_Awake() );
  experiences.push_back( new Experience_ShowTime() );
  experiences.push_back( new Experience_Sleep() );
  experiences.push_back( new Experience_SetTime() );
  experiences.push_back( new Experience_Mystic() );

  // CatsPlay,
  // MysticCat,
  // EyesFollowFinger,
  // ParallaxCat,
  // Shaken,
  // SwipeFinger,
  // Chastise
 
  // Add other experiences here
}

void Inveigle::begin() 
{
  currentState = STOPPED;

  sleepStarting = false;

  msitime = millis();
  msiwhen = random( 5000, 10000 );
}

void Inveigle::startExperience( int exper )
{
  currentExperience = experiences[ exper ];
  currentExperience->init();
  currentState = SETUP;
}

void Inveigle::setCurrentState( State state )
{
  currentState = state;
}

int Inveigle::getCurrentState()
{
  return currentState;
}

void Inveigle::operateExperience()
{
  switch ( currentState ) 
  {
    case SETUP:
      currentExperience->setup();
      if ( currentExperience->isSetupComplete() )
      {
        currentState = RUN;
      }
      break;

    case RUN:
      currentExperience->run();
      if ( currentExperience->isRunComplete() ) 
      {
        currentState = TEARDOWN;
      }
      break;

    case TEARDOWN:
      currentExperience->teardown();
      if ( currentExperience->isTeardownComplete() ) 
      {
        currentState = STOPPED;
      }
      break;

    case STOPPED:
      break;
  }
}

void Inveigle::loop() 
{
  // Give time to running experience
  if ( currentState != STOPPED )
  {
    operateExperience();
  }

  // Respond to Gestures
  

  // Set time


  //startExperience( Inveigle::SetTime );  // Settime experience
  

  // Detect deep sleep

  if ( sleepStarting == false )
  {
    if ( tof.cancelGestureDetected() )
    {
      setCurrentState( TEARDOWN );  // Put in teardown state
      sleepStarting = true;
    }
  }

  if ( sleepStarting )
  {
    //Wait until current experience stops

    if ( getCurrentState() == STOPPED )
    {
      startExperience( Inveigle::Sleep );   // Sleep experience
    }
  }

  /*

  // if no experience, start ShowTime

  if ( ( ! sleepStarting ) && ( getCurrentState() == STOPPED ) && ( video.getStatus() == 0 ) )
  {
    if ( millis() - msitime > msiwhen ) 
    {
      msitime = millis();
      msiwhen = random( 3000, 15000 );  

      Serial.print( "Starting experience " );
      Serial.println( Inveigle:: );

      startExperience( Inveigle::ShowTime );  // Settime experience
    }
  }

  */

  /*

  // Notifications, for example low-battery

  //checkBatteryStatus();

  Coming soon: Machine learning around running random experiences
  For now it's only the Experience_SetTime

  */

}
