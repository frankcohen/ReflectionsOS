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
#include "Experience_EasterEggFrank.h"
#include "Experience_EasterEggTerri.h"

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

  makeExp = new Experience_EasterEggFrank();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_EasterEggFrank" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_EasterEggTerri();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "ExperienceService error making Experience_EasterEggTerri" ) );
    while(1);
  }
  experiences.push_back( makeExp );
}

String ExperienceService::experienceNameToString( int experience ) 
{
  switch (experience) 
  {
    case Awake: return F("Awake");
    case ShowTime: return F("ShowTime");
    case Sleep: return F("Sleep");
    case Chastise: return F("Chastise");
    case EyesFollowFinger: return F("EyesFollowFinger");
    case ParallaxCat: return F("ParallaxCat");
    case Hover: return F("Hover");
    case CatsPlay: return F("CatsPlay");
    case MysticCat: return F("MysticCat");
    case Shaken: return F("Shaken");
    case GettingSleepy: return F("GettingSleepy");
    case Pounce: return F("Pounce");
    case EasterEggFrank: return F("EasterEggFrank");
    case EasterEggTerri: return F("EasterEggTerri");
    default: return F("Unknown Experience");
  }
}

void ExperienceService::begin() 
{
  currentState = STOPPED;

  gestureTimer = millis();

  catsplayTimer = millis();
  catsplayTimer2 = millis();

  experienceIndex = 0;

  temptimer = millis();

  mostrecentendtime = millis();
}

void ExperienceService::startExperience( int exper )
{
  expernum = exper;

  if (exper >= 0 && exper < experiences.size()) 
  {
    currentExperience = experiences[exper];
  }
  else 
  {
    Serial.print( "Invalid experience index: " );
    Serial.println(exper);
    return;
  }
  
  if ( currentExperience == nullptr )
  {
    Serial.print( "ExperienceService null pointer on starting experience " );
    Serial.println( exper );
    return;
  }
  
  /*
  Serial.print( F("startExperience ") );
  Serial.println( experienceNameToString( exper ) );
  */

  experiencestats.record( experienceNameToString( exper ) );

  currentExperience->init();
  currentState = SETUP;
}

int ExperienceService::getExperNum()
{
  return expernum;
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
      watchfacemain.clearSleepy();

      if ( currentExperience == NULL )
      {
        Serial.println( F("ExperienceService setup currentExperience is null"));
        while(1);
      }
      
      currentExperience->setup();
      if ( currentExperience->isSetupComplete() )
      {
        currentState = RUN;
      }

      break;

    case RUN:
      if ( currentExperience == NULL )
      {
        Serial.println( F("ExperienceService run currentExperience is null"));
        while(1);
      }

      currentExperience->run();

      // Single or double cancels the experience
      if ( accel.getSingleTapNoClear() || accel.getDoubleTapNoClear() )
      {
        String exn = currentExperience->getExperienceName();
        if ( ! ( exn == catsplayname || exn == PounceName ) )
        {
          Serial.println( "Experience tap exit" );
          currentExperience->setRunComplete( true );
          currentState = TEARDOWN;
          break;
        }
      } 

      if ( currentExperience->isRunComplete() ) 
      {
        currentState = TEARDOWN;
      }
      break;

    case TEARDOWN:

      if ( currentExperience == nullptr )
      {
        Serial.println( F("ExperienceService teardown currentExperience is null"));
      }
      
      currentExperience->teardown();

      if ( currentExperience->isTeardownComplete() ) 
      {
        currentState = STOPPED;
        watchfacemain.setDrawItAll();
        tof.resetGesture();
        accel.resetTaps();
      }
      break;

    case STOPPED:    
      accel.resetTaps();  // Clear any taps
      tof.getGesture();   // Clear any gestures
      mostrecentendtime = millis();
      break;
  }
}

bool ExperienceService::active()
{
  if ( getCurrentState() != STOPPED ) return true;
  return false;
}

bool ExperienceService::timeToRunAnother()
{
  if ( millis() - mostrecentendtime > 15000 )
  {
    return true;
  }
  return false;
}

void ExperienceService::loop()
{
  operateExperience();      // Run the current experience, if any
  experiencestats.update();
  
}
