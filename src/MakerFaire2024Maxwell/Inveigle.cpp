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
extern TimeService timeservice;
extern BLEServerClass bleServer;
extern BLEClientClass bleClient;

Inveigle::Inveigle() : currentExperience( nullptr ), currentState( STOPPED ) 
{
  // Add instances of each experience to the vector, do not change the order

  // TOF experiences
  
  makeExp = new Experience_Awake();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Awake" ) );
    while(1);
  }
  experiences.push_back( makeExp );
  
  makeExp = new Experience_ShowTime();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_ShowTime" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Sleep();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Sleep" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_SetTime();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_SetTime" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Chastise();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Chastise" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Eyes();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Eyes" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Parallax();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Parallax" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Swipe();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Swipe" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  // BLE experiences

  makeExp = new Experience_CatsPlay();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_CatsPlay" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  // Accel experiences

  makeExp = new Experience_MysticCat();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_MysticCat" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Shaken();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Shaken" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_GettingSleepy();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_GettingSleepy" ) );
    while(1);
  }
  experiences.push_back( makeExp );

  makeExp = new Experience_Pounce();
  if ( makeExp == nullptr )
  {
    Serial.println( F( "Inveigle error making Experience_Pounce" ) );
    while(1);
  }
  experiences.push_back( makeExp );

}

void Inveigle::begin() 
{
  currentState = STOPPED;

  shakenTimer = millis();

  catsplayTimer = millis();
  catsplayTimer2 = millis();

  noopTimer = millis();
  noopFlag = false;

  experienceIndex = 0;

  //startExperience( Inveigle::Awake );   // Sleep experience
}

void Inveigle::startExperience( int exper )
{
  currentExperience = experiences[ exper ];
  
  if ( currentExperience == nullptr )
  {
    Serial.print( "Inveigel null pointer on starting experience " );
    Serial.println( exper );
    return;
  }

  Serial.print( "Inveigle startExperience " );
  Serial.println( exper );

  currentExperience->init();
  currentState = SETUP;

  noopFlag = true;
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
      if ( currentExperience == NULL )
      {
        Serial.println( "Inveigle setup currentExperience is null");
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
        Serial.println( "Inveigle run currentExperience is null");
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
        Serial.println( "Inveigle teardown currentExperience is null");
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

void Inveigle::loop()
{
  operateExperience();      // Run the current experience, if any

  if ( getCurrentState() != STOPPED ) return;

  /*
  // ShowTime from accel next gesture

  if ( accel.getRecentGesture() != 0 )
  {
    startExperience( Inveigle::ShowTime );
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

    startExperience( Inveigle::CatsPlay );
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
            startExperience( Inveigle::ShowTime );
            Serial.println( "ShowTime" );
            break;
          case 2:
            startExperience( Inveigle::ShowTime );
            Serial.println( "ShowTime" );
            break;
          case 3:
            startExperience( Inveigle::Chastise ); 
            Serial.println( "Chastise" );
            break;
          case 4:
            startExperience( Inveigle::Shaken );
            Serial.println( "Shaken" );
            break;
          case 5:
            startExperience( Inveigle::SwipeFinger );
            Serial.println( "Shaken" );
            break;
          case 6:
            startExperience( Inveigle::ParallaxCat );
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
    startExperience( Inveigle::Sleep );   // Sleep experience and puts hardware into deep sleep    
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
          startExperience( Inveigle::Shaken );
          Serial.println( "Shaken" );
          break;
        case 2:
          startExperience( Inveigle::SwipeFinger );
          Serial.println( "Swipe" );
          break;
        case 3:
          startExperience( Inveigle::Chastise );
          Serial.println( "Chastise" );
          break;
        case 4:
          startExperience( Inveigle::Shaken );
          Serial.println( "Shaken" );
          break;
      }

    return;
    }
  }
}
