/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

3D wallpaper with parallax effect
Move the device, accelerometer senses position, changes wallpaper

Requires 6 jpg images to be named /REFLECTIONS/cat1_parallax_baseline.jpg, 2, 3, etc.
Images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

Uses the X axis of the Reflections board accellerometer to determine the image.
Future expansion should incorporate the Y and Z axis.

*/

#include "Experience_Parallax.h"

void Experience_Parallax::init()
{
  vidflag = true;  
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  stopped = false;
  idle = false;

  pictureNum = 6;
  parallaxWaitTime = millis();
  paralaxDuration = millis();
} 

void Experience_Parallax::setup() 
{
  if ( vidflag )
  {
    Serial.println( parallaxname );
    Serial.println( "SETUP" );

    video.startVideo( ParallaxCat_video );

    timeflag = true;
    vidflag = false;
    tearflag = true;
  }

  if ( video.getVideoTime() > 2600 )
  {
    eyestime = millis();
    dur = random( 1, 5 );

    paralaxDuration = millis();
    parallaxWaitTime = millis();

    video.setPaused( true );

    setSetupComplete( true );  // Signal that setup is complete
  }
}

void Experience_Parallax::run() 
{
  if ( millis() - paralaxDuration < ( 20000 + ( dur * 250 ) ) )
  { 
    if ( ( millis() - parallaxWaitTime ) < 1000 ) return;
    parallaxWaitTime = millis();

    float nx = accel.getXreading() + 1;

    Serial.println( nx );

    int mapped = map( nx, -1500, 1500, 1, 9);

    String mef = "cat";
    mef += String( mapped );
    mef += "_parallax_baseline.jpg";

    //Serial.println( mef );

    watchfacemain.drawImageFromFile( mef, true, 0, 0 ); 
    watchfacemain.show();
  }
  else
  {
    video.setPaused( false );

    if ( video.getStatus() == 0 )
    {
      setRunComplete(true);  // Signal run complete
    }
  }  

}

void Experience_Parallax::teardown() 
{
  if ( tearflag )
  {
    tearflag = false;
    Serial.print( parallaxname );
    Serial.println( "TEARDOWN" );
  }

  if ( video.getStatus() == 0 )
  {
    timeflag = true;
    setTeardownComplete( true );  // Signal teardown complete
  }
}

