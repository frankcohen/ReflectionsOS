/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

ExperienceService runs entertainment experiences.

Experiences.cpp is an abstract base class to implement the ExperienceService actions.
Each experience has a startup, run, and teardown method. ExperienceService uses gesture,
time, and event sensing to run an experience. It runs the startup and run
methods. It uses gesture and event sensing to interrupt the run method and
skip to the teardown method. 

Connect experiences to the ExperienceService by adding an Experience implementation to 
the #include list and class instantiation method below.

*/

#include "ExperienceService.h"

// Add an include for each experience here
#include "Experience_Awake.h"
#include "Experience_ShowTime.h"
#include "Experience_Sleep.h"
#include "Experience_Chastise.h"
#include "Experience_Eyes.h"
#include "Experience_Parallax.h"
#include "Experience_Hover.h"
#include "Experience_MysticCat.h"
#include "Experience_CatsPlay.h"
#include "Experience_Shaken.h"
#include "Experience_GettingSleepy.h"
#include "Experience_Pounce.h"

ExperienceService::ExperienceService() : currentExperience( nullptr ), currentState( STOPPED ) 
{
  // Add instances of each experience to the vector, do not change the order

  // TOF experiences
  
  makeExp = new Experience_Awake();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Awake" ) );
    while(1);
  }
  experiences.push_back( makeExp );
  
  makeExp = new Experience_ShowTime();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_ShowTime" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Sleep();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Sleep" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Chastise();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Chastise" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Eyes();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Eyes" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Parallax();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Parallax" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Hover();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Hover" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  // BLE experiences

  makeExp = new Experience_CatsPlay();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_CatsPlay" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  // Accel experiences

  makeExp = new Experience_MysticCat();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_MysticCat" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Shaken();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Shaken" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_GettingSleepy();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_GettingSleepy" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Pounce();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_Pounce" ) );
    while(1);
  }
  experiences.push_back( makeExp );

}

void ExperienceService::begin() 
{
  currentState = STOPPED;

  gestureTimer = millis();

  catsplayTimer = millis();
  catsplayTimer2 = millis();
  afterTimer = millis();

  noopTimer = millis();
  noopFlag = false;

  experienceIndex = 0;

  afterCatsPlay = millis();
  afterPounce = millis();

  //startExperience( ExperienceService::Awake );   // Sleep experience
}

void ExperienceService::resetAfterTimer()
{
  afterTimer = millis();
}

void ExperienceService::startExperience( int exper )
{
  currentExperience = experiences[ exper ];
  
  if ( currentExperience == nullptr )
  {
    Serial.print( "Inveigel null pointer on starting experience " );
    Serial.println( exper );
    return;
  }

  Serial.print( "startExperience " );
  Serial.println( exper );

  currentExperience->init();
  currentState = SETUP;

  noopFlag = true;
}

void ExperienceService::setCurrentState( State state )
{
  currentState = state;
}

int ExperienceService::getCurrentState()
{
  return currentState;
}

void ExperienceService::operateExperience()
{
  switch ( currentState ) 
  {
    case SETUP:
      if ( currentExperience == NULL )
      {
        Serial.println( "ExperienceService setup currentExperience is null");
        while(1);
      }

      currentExperience->setup();
      if ( currentExperience->isSetupComplete() )
      {
        currentState = RUN;
      }

      tof.stopGestureSensing();

      break;

    case RUN:
      if ( currentExperience == NULL )
      {
        Serial.println( "ExperienceService run currentExperience is null");
        while(1);
      }

      currentExperience->run();

      if ( currentExperience->isRunComplete() ) 
      {
        currentState = TEARDOWN;
      }
      break;

    case TEARDOWN:

      if ( currentExperience == nullptr )
      {
        Serial.println( "ExperienceService teardown currentExperience is null");
      }
      
      currentExperience->teardown();

      if ( currentExperience->isTeardownComplete() ) 
      {
        currentState = STOPPED;

        tof.startGestureSensing();
        resetAfterTimer();        
      }
      break;

    case STOPPED:    
      break;
  }
}

bool ExperienceService::active()
{
  if (  getCurrentState() != STOPPED ) return true;
  return false;
}

void ExperienceService::loop()
{
  operateExperience();      // Run the current experience, if any

  // If in Cats Play, triple tap, and heading towards this device, then Pounce
  // Detect triple tap
  // Calculate heading towards this device
  // Pounce



  if ( getCurrentState() != STOPPED ) return;

  if ( video.getStatus() != 0 ) return;

  if ( millis() - afterTimer < 4000 ) return;

  if ( millis() - gestureTimer > 500 )
  {
    gestureTimer = millis();

    // Start a new experience from the TOF sensor

    tof.startGestureSensing();

    TOF::TOFGesture recentGesture = tof.getGesture();

    switch ( recentGesture )
    {
      case TOF::TOFGesture::None:
        break;

      case TOF::TOFGesture::Left:
        //startExperience( ExperienceService::Chastise );
        //startExperience( ExperienceService::EyesFollowFinger );

      case TOF::TOFGesture::Circular:
        startExperience( ExperienceService::MysticCat );

      case TOF::TOFGesture::Sleep:

      case TOF::TOFGesture::Right:
        // ShowTime with fun messages
        //startExperience( ExperienceService::ShowTime );

      case TOF::TOFGesture::Up:
        // Parallax cat
        //startExperience( ExperienceService::ParallaxCat );

      case TOF::TOFGesture::Hover:
        // Finger hovers in one spot
        //startExperience( ExperienceService::Hover );

      case TOF::TOFGesture::Down:

      default:
        Serial.print( "Unknown TOF experience ");
        Serial.println( recentGesture );
        break;
    }

    // Shake experience

    if ( accel.shaken() )
    {
      //startExperience( ExperienceService::Shaken );
    }

    // Cats Play

    /*
    When another cat is in BLE range, go into Cats Play for the next 2 minutes
    Show Cats Play video, estimating diraction
    */

    String mef = "BLE count ";
    mef += blesupport.getRemoteDevicesCount();
    Serial.println( mef );


    if ( ( blesupport.getRemoteDevicesCount() > 0 ) && ( ( millis() - afterCatsPlay) > ( 60 * 1000 ) ) ) 
    {
      afterCatsPlay = millis();

      startExperience( ExperienceService::CatsPlay );
    }
    
    


    /*

    React to pounces from other cats facing your direction, play pounce experience

    When in Cats Play, triple tap sends a pounce

    */



    // Hover

    // GettingSleepy
    
    // Sleep gesture

    // Swipe










  }
}
