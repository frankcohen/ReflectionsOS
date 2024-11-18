/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

AnimationService runs entertainment experiences.

Experiences.cpp is an abstract base class to implement the AnimationService actions.
Each experience has a startup, run, and teardown method. AnimationService uses gesture,
time, and event sensing to run an experience. It runs the startup and run
methods. It uses gesture and event sensing to interrupt the run method and
skip to the teardown method. 

Connect experiences to the AnimationService by adding an Experience implementation to 
the #include list and class instantiation method below.

*/

#include "AnimationService.h"

// Add an include for each experience here
#include "Experience_Awake.h"
#include "Experience_ShowTime.h"
#include "Experience_Sleep.h"
#include "Experience_SetTime.h"
#include "Experience_Chastise.h"
#include "Experience_Eyes.h"
#include "Experience_Parallax.h"
#include "Experience_Swipe.h"
#include "Experience_MysticCat.h"
#include "Experience_CatsPlay.h"
#include "Experience_Shaken.h"
#include "Experience_GettingSleepy.h"
#include "Experience_Pounce.h"

extern Video video;
extern TextMessageService textmessageservice;
extern BLEServerClass bleServer;
extern BLEClientClass bleClient;

AnimationService::AnimationService() : currentExperience( nullptr ), currentState( STOPPED ) 
{
  // Add instances of each experience to the vector, do not change the order

  // TOF experiences
  
  makeExp = new Experience_Awake();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Awake" ) );
    while(1);
  }
  experiences.push_back( makeExp );
  
  makeExp = new Experience_ShowTime();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_ShowTime" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Sleep();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Sleep" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_SetTime();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_SetTime" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Chastise();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Chastise" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Eyes();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Eyes" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Parallax();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Parallax" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Swipe();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Swipe" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  // BLE experiences

  makeExp = new Experience_CatsPlay();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_CatsPlay" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  // Accel experiences

  makeExp = new Experience_MysticCat();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_MysticCat" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Shaken();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Shaken" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_GettingSleepy();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_GettingSleepy" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Pounce();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "AnimationService error making Experience_Pounce" ) );
    while(1);
  }
  experiences.push_back( makeExp );

}

void AnimationService::begin() 
{
  currentState = STOPPED;

  shakenTimer = millis();

  catsplayTimer = millis();
  catsplayTimer2 = millis();

  noopTimer = millis();
  noopFlag = false;

  experienceIndex = 0;

  //startExperience( AnimationService::Awake );   // Sleep experience
}

void AnimationService::startExperience( int exper )
{
  currentExperience = experiences[ exper ];
  
  if ( currentExperience == nullptr )
  {
    Serial.print( "Inveigel null pointer on starting experience " );
    Serial.println( exper );
    return;
  }

  Serial.print( "AnimationService startExperience " );
  Serial.println( exper );

  currentExperience->init();
  currentState = SETUP;

  noopFlag = true;
}

void AnimationService::setCurrentState( State state )
{
  currentState = state;
}

int AnimationService::getCurrentState()
{
  return currentState;
}

void AnimationService::operateExperience()
{
  switch ( currentState ) 
  {
    case SETUP:
      if ( currentExperience == NULL )
      {
        Serial.println( "AnimationService setup currentExperience is null");
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
        Serial.println( "AnimationService run currentExperience is null");
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
        Serial.println( "AnimationService teardown currentExperience is null");
      }
      
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

void AnimationService::loop()
{
  operateExperience();      // Run the current experience, if any

  if ( getCurrentState() != STOPPED ) return;

  /*
  // ShowTime from accel next gesture

  if ( accel.getRecentGesture() != 0 )
  {
    startExperience( AnimationService::ShowTime );
    return;
  }
  */

  // Run CatsPlay Experience when RSSI says they are close

  int mrs = bleClient.getDistance();

  if ( ( mrs > catsplayCloser ) && ( mrs != 0 ) ) 
  {
    catsplayTimer = millis();

    Serial.print("Device is close. RSSI = " );
    Serial.println( mrs );

    startExperience( AnimationService::CatsPlay );
    return;
  }

  // Getting sleepy after 1 minute of not operations

  if ( ( millis() - noopTimer ) > ( 1000 * 30 ) )
  {
    noopTimer = millis();

    if ( 1 )   // ! noopFlag
    {
      if ( video.getStatus() == 0 )
      {
        switch ( random( 1, 7 ) )
        {
          case 1:
            startExperience( AnimationService::ShowTime );
            Serial.println( "ShowTime" );
            break;
          case 2:
            startExperience( AnimationService::ShowTime );
            Serial.println( "ShowTime" );
            break;
          case 3:
            startExperience( AnimationService::Chastise ); 
            Serial.println( "Chastise" );
            break;
          case 4:
            startExperience( AnimationService::Shaken );
            Serial.println( "Shaken" );
            break;
          case 5:
            startExperience( AnimationService::SwipeFinger );
            Serial.println( "Shaken" );
            break;
          case 6:
            startExperience( AnimationService::ParallaxCat );
            Serial.println( "Shaken" );
            break;
        }
      }
    }

    noopFlag = false;
    return;
  }

return;

/*
  if ( mygs == TOF::Sleep ) 
  {
    Serial.println( "sleep");
    startExperience( AnimationService::Sleep );   // Sleep experience and puts hardware into deep sleep    
    return;
  }
*/

  int mygs = tof.getGesture();

  if ( video.getStatus() == 0 )
  {
    if ( millis() - shakenTimer > 20000 )
    {
      shakenTimer = millis();

      Serial.print( "TOF gesture detected " );
      if ( mygs == TOF::BombDrop ) Serial.println( "bombdrop" );
      if ( mygs == TOF::Circular ) Serial.println( "circular" );
      if ( mygs == TOF::FlyAway ) Serial.println( "flyaway" );
      if ( mygs == TOF::Horizontal ) Serial.println( "horizontal" );
      if ( mygs == TOF::Vertical ) Serial.println( "vertical" );

      switch ( random( 1, 5 ) )
      {
        case 1:
          startExperience( AnimationService::Shaken );
          Serial.println( "Shaken" );
          break;
        case 2:
          startExperience( AnimationService::SwipeFinger );
          Serial.println( "Swipe" );
          break;
        case 3:
          startExperience( AnimationService::Chastise );
          Serial.println( "Chastise" );
          break;
        case 4:
          startExperience( AnimationService::Shaken );
          Serial.println( "Shaken" );
          break;
      }

    return;
    }
  }
}
