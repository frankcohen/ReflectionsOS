/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience lets the user set the time for the internal RTC

*/

#include "Experience_MysticCat.h"

void Experience_MysticCat::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;
  vidflag = true;  
} 

void Experience_MysticCat::setup() 
{
  if ( vidflag )
  {
    Serial.print( mysticname );
    Serial.println( F("SETUP") );

    video.startVideo( MysticCat_video );

    timeflag = true;
    vidflag = false;
    tearflag = true;
  }

  if ( video.getVideoTime() > 3000 )
  {
    video.setPaused( true );
    setSetupComplete( true );  // Signal that setup is complete
  }
}

void Experience_MysticCat::run() 
{
  if ( timeflag )
  {
    int pIndex = random(0, 2);
    int sIndex = random(0, 10);
    
    if (pIndex == 0)
    {
      theMsg1 = affirmative[ sIndex ][ 0 ];
      theMsg2 = affirmative[ sIndex ][ 1 ];
    }
    
    if (pIndex == 1)
    {
      theMsg1 = noncommital[ sIndex ][ 0 ];
      theMsg2 = noncommital[ sIndex ][ 1 ];
    }

    if (pIndex == 2)
    {
      theMsg1 = negative[ sIndex ][ 0 ];
      theMsg2 = negative[ sIndex ][ 1 ];
    }

    textmessageservice.startShow( TextMessageExperiences::MysticalAnswer, theMsg1, theMsg2 );  
    timeflag = false;
  }
  else
  {
    if ( ! textmessageservice.active() )
    {
      video.setPaused( false );
      setRunComplete(true);  // Signal run complete
      return;
    }
  }

  if ( video.getStatus() == 0 )
  {
    setRunComplete(true);  // Signal run complete
    return;
  }
}

void Experience_MysticCat::teardown() 
{
  if ( tearflag )
  {
    tearflag = false;
    Serial.print( mysticname );
    Serial.println( F("TEARDOWN") );
  }

  if ( video.getStatus() == 0 )
  {
    timeflag = true;
    setTeardownComplete( true );  // Signal teardown complete
  }
}