/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires PNGdec library https://github.com/bitbank2/PNGdec

*/

/*
WatchFace Main todo:

DONE Main face opener video - hands move in both directions, cat appears smile first
DONE Display correct hours and minutes from RTC
Download new blink graphics
DONE Display main face with speed (mjpeg's at 8 FPS, so should you)
  DONE The strategy: skip the start, clear the buffer when the blink is done, draw the elements when they changed
DONE Remove horizontal lines from top and bottom of cat face pngs
Single tap to move from MainPanel, TimePanel, HealthPanel, TimerPanel, movement animation, haptic for single and double tap
Animate rotation using mjpeg for speed/smoothness
Animate each rotation
Double tap in TimePanel to see SetTimePanel
SetTimePanel left-right tilt for minutes, top-bottom tilt for hours, wait 3 seconds to accept and see MainPanel, larger font, hours/minute animation
HealthPanel double tap to clear steps, see MainPanel
TimerPanel shows countdown, double tap to clear, single tap for MainPanel,
   left-right tilt to choose timer length (5, 10, 20, 30, 60, 90), wait 3 seconds to start and see MainPanel

*/

#include "WatchFaceMain.h"

WatchFaceMain::WatchFaceMain() 
    : WatchFaceBase() // Call to the base class constructor
{}

void WatchFaceMain::begin() 
{
  WatchFaceBase::begin();  // This ensures the base method is executed

  // video.startVideo( WatchFaceOpener_video );

  maintimer = millis();

  drawitall = true;

  catFaceIndex = 1;
  catFaceTimer = millis();
  catFaceDirection = true;
  catFaceWait = rand() % 30000;

  battimer = millis();
  batcount = 0;
  batlev = 0;

  oldHour = 0;
  oldMinute = 0;
  oldBattery = 0;
  oldBlink = 0;

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  panel = STARTUP;
  video.startVideo( WatchFaceOpener_video );

  myTeeTime = millis();
}

// Battery indicator grows/loses leaves

void WatchFaceMain::updateBattery()
{
  if ( ( millis() - battimer ) > 2000 )
  {
    battimer = millis();

    batlev = battery.getBatteryLevel();
    if ( ( batlev != batcount ) && ( batcount < 4 ) ) 
    {
      batcount++;
      displayUpdateable = true;      
    }
  }
}

// Hour and minutes hands to current time

void WatchFaceMain::updateHoursAndMinutes()
{
  int currentHour2 = realtimeclock.getCurrentHourFromRTC();

  if ( currentHour2 == -1 ) currentHour2 = 2;
  
  if ( currentHour2 != currentHour )
  {
    currentHour = currentHour2;
    displayUpdateable = true;
    drawitall = true;
  }

  int currentMinute2 = realtimeclock.getCurrentMinuteFromRTC();

  if ( currentMinute2 == -1 ) currentMinute2 = 15;

  if ( currentMinute2 != currentMinute )
  {
    currentMinute = currentMinute2;
    displayUpdateable = true;
    drawitall = true;
  }

}

// Cat blinks eyes

void WatchFaceMain::updateBlink()
{
  if ( ! blinking )
  {
    if ( ( millis() - catFaceTimer ) > catFaceWait )
    {
      catFaceTimer = millis();
      blinking = true;
      catFaceDirection = true;
    }
  }
  else
  {
    if ( catFaceDirection )
    {
      catFaceIndex++;
      displayUpdateable = true;
  
      if ( catFaceIndex > 5 )
      {
        catFaceDirection = false;       
        catFaceIndex = 5;
      }
    }
    else
    {
      catFaceIndex--;
      displayUpdateable = true;
  
      if ( catFaceIndex < 1 )
      {
        // Generate a random number between 5000 and 10000
        catFaceDirection = true;       
        catFaceIndex = 1;
        blinking = false;
        catFaceTimer = millis();
        catFaceWait = rand() % 30000;
        drawitall = true;

        //Serial.print( "catFaceWait ");
        //Serial.println( catFaceWait );
      }
    }
  }
}

// Draw the elements to the buffer

void WatchFaceMain::updateDisplayMain()
{
  if ( ! displayUpdateable ) return;
  displayUpdateable = false;

  /*
  unsigned long mytime = millis();
  unsigned long mytime2 = millis();
  */

  if ( drawitall ) start();    // Clear frame buffer

  /*
  Serial.print( "a: " );
  Serial.print( millis() - mytime );
  mytime = millis();
  */

  // Background

  if ( drawitall ) drawImageFromFile( wfMainBackground, true, 0, 0 );

  /*
  Serial.print( " b: " );
  Serial.print( millis() - mytime );
  mytime = millis();
  */

  String mef;

  // Draw hour image
  if ( ( currentHour != oldHour ) || drawitall )
  {
    oldHour = currentHour;

    mef = wfMainHours;
    mef += currentHour;
    mef += wfMainHours2;
    drawImageFromFile( mef, true, 0, 0 );
  } 

  /*
  Serial.print( " c: " );
  Serial.print( millis() - mytime );
  mytime = millis();
  */

  int minute = map( currentMinute, 1, 60, 1, 20) + 1;
  if ( minute > 20 ) minute == 20;

  // Draw minutes image
  if ( ( currentMinute != oldMinute ) || drawitall )
  {
    oldMinute = currentMinute;

    mef = wfMainMinutes;
    mef += minute ;
    mef += wfMainMinutes2;
    drawImageFromFile( mef, true, 0, 0 );
  }

  /*
  Serial.print( " d: " );
  Serial.print( millis() - mytime );
  mytime = millis();
  */

  // Battery indicator

    if ( ( batcount != oldBattery ) || drawitall )
    {
      oldBattery = batcount;

      mef = wfMainBattery;
      mef += batcount + 1;
      mef += wfMainBattery2;
      drawImageFromFile( mef, true, 0, 0 );
    }

    /*
    Serial.print( " e: " );
    Serial.print( millis() - mytime );
    mytime = millis();
    */

  // Cat face in the middle, and he blinks randomly

    if ( ( catFaceIndex != oldBlink ) || drawitall )
    {
      oldBlink = catFaceIndex;

      mef = wfMainFace;
      mef += catFaceIndex;
      mef += wfMainFace2;
      drawImageFromFile( mef, true, 0, 0 );
    }

    /*
    Serial.print( " f: " );
    Serial.print( millis() - mytime );
    mytime = millis();
    */

    show();

    /*
    Serial.print( " g: " );
    Serial.print( millis() - mytime );
    mytime = millis();

    Serial.print( " x: " );
    Serial.println( millis() - mytime2 );
    */

    drawitall = false;

}

// Draws hour glass time left display, 3, 2, 1
// Returns true when time is up

bool WatchFaceMain::updateTimeLeft()
{
  int index = 1;

  if ( noMovementTime > 3000 )
  {
    index = 2;
  }
  if ( noMovementTime > 6000 )
  {
    index = 3;
  }
  if ( noMovementTime > 9000 )
  {
    index = 4;
  }
  if ( noMovementTime > 12000 )
  {
    return true;
  }

  mef = wfMainHourglass;
  mef += index;
  mef += wfMainHourglass2;

  drawImageFromFile( mef, true, 0, 0 );

  return false;
}

void WatchFaceMain::loop()
{
  /*
  Serial.print( "* " );
  Serial.println( millis() - myTeeTime );
  myTeeTime = millis();
*/

  if ( rotating > 0 )
  {
    if ( millis() - maintimer > flipspeed )
    {
      maintimer = millis();

      rotating++;

      if ( rotating >= wfMainMaxFlips ) 
      {
        rotating = 0;
        return;
      }

      if ( panel = MAIN )
      {
        mef = wfMainFlip;
      }
      else
      {
        mef = wfMainFaceBlue;
      }

      mef += rotating;
      mef += wfMainFlip2;
      drawImageFromFile( mef, true, 0, 0 );
      show();
    }

    return;
  }
 
  switch ( panel ) 
  {
    case STARTUP:

      if ( ! video.getStatus() )
      {
        panel = MAIN;
        maintimer = millis();
        break;
      }

      break;

    case MAIN:

      if ( millis() - maintimer > 50 )
      {
        maintimer = millis();

        updateBlink();            // Blink eyes
        updateBattery();          // Battery level indicator
        updateHoursAndMinutes();  // Update hour and minute hands
      }

      updateDisplayMain();

      if ( accel.tapped() )
      {
        panel = DISPLAYING_DIGITAL_TIME;
        needssetup = true;
        rotating = 1;
      }

      break;

    case DISPLAYING_DIGITAL_TIME:

      if ( needssetup )
      {
        Serial.println( "DISPLAYING_DIGITAL_TIME" );
        needssetup = false;
      }

      if ( accel.tapped() )
      {
        panel = DISPLAYING_HEALTH_STATISTICS;
        rotating = 1;
        needssetup = true;
      }

      if ( accel.doubletapped() )
      {
        panel = SETTING_DIGITAL_TIME;
        rotating = 1;
        needssetup = true;
        noMovementTime = millis();
      }
      
      break;

    case SETTING_DIGITAL_TIME:

      if ( needssetup )
      {
        Serial.println( "SETTING_DIGITAL_TIME" );
        needssetup = false;
      }

      if ( updateTimeLeft() )
      {
        panel = MAIN;
        rotating = 1;
      }

      break;

    case DISPLAYING_HEALTH_STATISTICS:

      if ( needssetup )
      {
        Serial.println( "DISPLAYING_HEALTH_STATISTICS" );
        needssetup = false;
      }

      if ( accel.tapped() )
      {
        panel = DISPLAYING_TIMER;
        rotating = 1;
        needssetup = true;
      }

      if ( accel.doubletapped() )
      {
        panel = SETTING_HEALTH_STATISTICS;
        rotating = 1;
        needssetup = true;
        noMovementTime = millis();
      }

      break;

    case SETTING_HEALTH_STATISTICS:

      if ( needssetup )
      {
        Serial.println( "SETTING_HEALTH_STATISTICS" );
        needssetup = false;
      }

      if ( accel.doubletapped() )
      {
        // Reset health stats here


        // then back to MAIN

        panel = MAIN;
        rotating = 1;
        needssetup = true;
      }

      if ( updateTimeLeft() )
      {
        panel = MAIN;
        rotating = 1;
      }

      break;

    case DISPLAYING_TIMER:

      if ( needssetup )
      {
        Serial.println( "DISPLAYING_TIMER" );
        needssetup = false;
      }

      if ( accel.tapped() )
      {

        // Reset timer here


        panel = MAIN;
        rotating = 1;
      }

      if ( accel.doubletapped() )
      {
        panel = SETTING_TIMER;
        rotating = 1;
        noMovementTime = millis();
      }

      if ( updateTimeLeft() )
      {
        panel = MAIN;
        rotating = 1;
      }

      break;

    case SETTING_TIMER:

      if ( needssetup )
      {
        Serial.println( "SETTING_TIMER" );
        needssetup = false;
      }

      if ( updateTimeLeft() )
      {
        // Start timer



        // and return to  Main

        panel = MAIN;
        rotating = 1;
      }

      break;

  }

}
