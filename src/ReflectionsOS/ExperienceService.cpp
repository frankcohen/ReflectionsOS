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
#include "Experience_Sand.h"

ExperienceService::ExperienceService() : currentExperience(nullptr), currentState(STOPPED) 
{
    // List of experience types to create
    const char* experienceNames[] = {
        "Experience_Awake", "Experience_ShowTime", "Experience_Sleep", "Experience_Chastise",
        "Experience_Eyes", "Experience_Parallax", "Experience_Hover", "Experience_CatsPlay",
        "Experience_MysticCat", "Experience_Shaken", "Experience_GettingSleepy", "Experience_Pounce",
        "Experience_EasterEggFrank", "Experience_EasterEggTerri",
        "Experience_Sand"
    };

    // Loop over the experience names, dynamically creating each experience and adding to the vector
    for (const char* expName : experienceNames) {
        Experience* makeExp = nullptr;
        
        if (strcmp(expName, "Experience_Awake") == 0) {
            makeExp = new Experience_Awake();
        } else if (strcmp(expName, "Experience_ShowTime") == 0) {
            makeExp = new Experience_ShowTime();
        } else if (strcmp(expName, "Experience_Sleep") == 0) {
            makeExp = new Experience_Sleep();
        } else if (strcmp(expName, "Experience_Chastise") == 0) {
            makeExp = new Experience_Chastise();
        } else if (strcmp(expName, "Experience_Eyes") == 0) {
            makeExp = new Experience_Eyes();
        } else if (strcmp(expName, "Experience_Parallax") == 0) {
            makeExp = new Experience_Parallax();
        } else if (strcmp(expName, "Experience_Hover") == 0) {
            makeExp = new Experience_Hover();
        } else if (strcmp(expName, "Experience_CatsPlay") == 0) {
            makeExp = new Experience_CatsPlay();
        } else if (strcmp(expName, "Experience_MysticCat") == 0) {
            makeExp = new Experience_MysticCat();
        } else if (strcmp(expName, "Experience_Shaken") == 0) {
            makeExp = new Experience_Shaken();
        } else if (strcmp(expName, "Experience_GettingSleepy") == 0) {
            makeExp = new Experience_GettingSleepy();
        } else if (strcmp(expName, "Experience_Pounce") == 0) {
            makeExp = new Experience_Pounce();
        } else if (strcmp(expName, "Experience_EasterEggFrank") == 0) {
            makeExp = new Experience_EasterEggFrank();
        } else if (strcmp(expName, "Experience_EasterEggTerri") == 0) {
            makeExp = new Experience_EasterEggTerri();
        } else if (strcmp(expName, "Experience_Sand") == 0) {
            makeExp = new Experience_Sand();
        }

        if (makeExp == nullptr) {
            Serial.printf("ExperienceService error making %s\n", expName);
            video.stopOnError( expName, F("unable"), F("to"), F("create"), F(" "));
        } else {
            experiences.push_back(makeExp);
        }
    }
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
    case Sand: return F("Sand");
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

bool ExperienceService::isRunningExperience()
{
  if ( currentState != STOPPED ) return true;
  return false;
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
    {
      currentExperience->setup();
      if ( currentExperience->isSetupComplete() )
      {
        currentState = RUN;
      }

      // Clear the sleep timer, unless this is the Sleep Experience

      String exn = currentExperience->getExperienceName();
      if ( exn != SleepName ) { watchfacemain.clearSleepy(); }

      textmessageservice.stop();
      
      if ( currentExperience == NULL )
      {
        Serial.println( F("ExperienceService setup currentExperience is null"));
        video.stopOnError( F( "currExp" ), F( "is null" ), F( " " ), F( " " ), F( " " ) );
      }      
    }
    break;
    
    case RUN:
    {
      if ( currentExperience == NULL )
      {
        Serial.println( F("ExperienceService run currentExperience is null"));
        video.stopOnError( F( "currExp" ), F( "is null" ), F( " " ), F( " " ), F( " " ) );
      }

      currentExperience->run();

      // Single or double cancels the experience
      if ( accel.getSingleTapNoClear() || accel.getDoubleTapNoClear() )
      {
        // Experiences that should NOT exit on tap
        String exn2 = currentExperience->getExperienceName();
        if ( ! ( exn2 == catsplayname ||
                exn2 == PounceName  ||
                exn2 == SleepName ) )
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
