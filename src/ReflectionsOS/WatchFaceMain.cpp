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
DONE Single tap to move from MainPanel, TimePanel, HealthPanel, TimerPanel, movement animation, haptic for single and double tap
DONE Animate each rotation
DONE Animate rotation using mjpeg for speed/smoothness
DONE Double tap in TimePanel to see SetTimePanel
DONE TextMessageService uses double buffer
DONE Timeout to main panel
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

  maintimer = millis();

  drawitall = true;

  catFaceIndex = 1;
  catFaceTimer = millis();
  catFaceDirection = true;
  catFaceWait = rand() % 30000;

  tilttimer = millis();
  oldtilthour = 0;
  oldtiltminute = 0;

  battimer = millis();
  batcount = 0;
  batlev = 0;

  oldHour = 0;
  oldMinute = 0;
  oldBattery = 5;
  oldBlink = 0;

  referenceY = accel.getYreading() + 15000; // Current Y-axis tilt value
  waitForNextReference = false; // Delay reference update after hour change
  lastChangeTime = 0; // Timestamp of the last hour change

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  panel = STARTUP;
  video.startVideo( WatchFaceOpener_video );
}

// Battery indicator grows/loses leaves

void WatchFaceMain::updateBattery()
{
  if ( ( millis() - battimer ) > 2000 )
  {
    battimer = millis();

    batlev = battery.getBatteryLevel();
    if ( ( batlev != batcount ) && ( batcount < 3 ) ) 
    {
      batcount++;
      displayUpdateable = true;      
    }
  }
}

// Hour and minutes hands to current time

void WatchFaceMain::updateHoursAndMinutes()
{
  int currentHour2 = realtimeclock.getHour();

  if ( currentHour2 == -1 ) currentHour2 = 2;
  if ( currentHour2 > 12 ) currentHour2 = currentHour2 - 12;
  
  if ( currentHour2 != currentHour )
  {
    currentHour = currentHour2;
    displayUpdateable = true;
    drawitall = true;
  }

  int currentMinute2 = realtimeclock.getMinute();

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

    if ( currentHour > 20 ) currentHour = 20;

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

bool WatchFaceMain::updateTimeLeftNoShow()
{
  if ( ( millis() - noMovementTime ) > 12000 )
  {
    return true;
  }

  return false;
}

bool WatchFaceMain::updateTimeLeft()
{
  int index = 1;

  if ( ( millis() - noMovementTime ) > 3000 )
  {
    index = 2;
  }

  if ( ( millis() - noMovementTime ) > 6000 )
  {
    index = 3;
  }

  if ( ( millis() - noMovementTime ) > 9000 )
  {
    index = 4;
  }

  if ( ( millis() - noMovementTime ) > 12000 )
  {
    return true;
  }

  mef = wfMainHourglass;
  mef += index;
  mef += wfMainHourglass2;

  drawImageFromFile( mef, true, 0, 0 );

  show();

  return false;
}

void WatchFaceMain::loop()
{
  
  if ( video.getStatus() )
  {
    return;
  }

  if ( textmessageservice.active() )
  {
    return;
  } 

  if ( experienceservice.active() )
  {
    return;
  } 

  switch ( panel ) 
  {
    case STARTUP:

      panel = MAIN;
      needssetup = true;
      maintimer = millis();
      haptic.playEffect(14);  // 14 Strong Buzz
      break;

    case MAIN:

      if ( needssetup )
      {
        Serial.println( "MAIN" );
        needssetup = false;
        return;
      }

      if ( millis() - maintimer > 50 )
      {
        maintimer = millis();

        updateBlink();            // Blink eyes
        updateBattery();          // Battery level indicator
        updateHoursAndMinutes();  // Update hour and minute hands
      }

      updateDisplayMain();

      if ( accel.tapped() || accel.doubletapped() )
      {
        panel = DISPLAYING_DIGITAL_TIME;
        haptic.playEffect(14);  // 14 Strong Buzz
        video.startVideo( WatchFaceFlip1_video );
        needssetup = true;
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      break;

    case DISPLAYING_DIGITAL_TIME:

      if ( needssetup )
      {
        Serial.println( "DISPLAYING_DIGITAL_TIME" );
        needssetup = false;
        drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
        textmessageservice.startShow( DigitalTimeFadeIn );                     // Show saying plus hour and minute
        noMovementTime = millis();
        return;
      }
/*
      if ( accel.tapped() )
      {
        panel = DISPLAYING_HEALTH_STATISTICS;
        haptic.playEffect(14);  // 14 Strong Buzz
        video.startVideo( WatchFaceFlip2_video );
        needssetup = true;
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }
*/
      if ( accel.tapped() )
      {
        panel = SETTING_DIGITAL_TIME;
        haptic.playEffect(14);  // 14 Strong Buzz
        video.startVideo( WatchFaceFlip2_video );
        needssetup = true;
        noMovementTime = millis();
        textmessageservice.stop();
        referenceY = accel.getYreading() + 15000; // Current Y-axis tilt value
      }
      
      // Update and display time

/*
      if ( updateTimeLeftNoShow() )
      {
        panel = MAIN;
        haptic.playEffect(14);  // 14 Strong Buzz
        video.startVideo( WatchFaceFlip3_video );
        textmessageservice.stop();
        needssetup = true;
        return;
      }
*/

      textmessageservice.updateTime();

      break;

    case SETTING_DIGITAL_TIME:

      if ( needssetup )
      {
        Serial.println( "SETTING_DIGITAL_TIME" );
        needssetup = false;
        drawImageFromFile( wfMain_SetTime_Background, true, 0, 0 );
        noMovementTime = millis();
        textmessageservice.stop();
        currentHour = realtimeclock.getHour();
        return;
      }

      // Change time by tilting left-right and up-down

      if ( millis() - tilttimer > 500 )
      {
        tilttimer = millis();

        float rawY = accel.getYreading() + 15000; // Current Y-axis tilt value
        float threshold = referenceY * 0.1;
        const unsigned long repeatDelay = 500; // Auto-repeat delay in milliseconds

        if ( waitForNextReference ) 
        {
          if (millis() - lastChangeTime >= 1000) 
          { // Wait 1 second before updating reference
            referenceY = rawY; // Update reference to new baseline
            waitForNextReference = false;
            Serial.print("Reference Y updated to: ");
            Serial.println(referenceY);
          }
        }

        if (rawY > referenceY + threshold) 
        {
          currentHour++;
          if (currentHour > 12) 
          {
            currentHour = 1; // Wrap around to 1 after 12
          }
          Serial.print("Hour increased to: ");
          Serial.println(currentHour);
          lastChangeTime = millis();
          lastRepeatTime = millis();
          waitForNextReference = true; // Start delay before updating reference
        }
  
        else if (rawY < referenceY - threshold) 
        {
          currentHour--;
          if (currentHour < 1) 
          {
            currentHour = 12; // Wrap around to 12 after 1
          }

          Serial.print("Hour decreased to: ");
          Serial.println(currentHour);
          lastChangeTime = millis();
          lastRepeatTime = millis();
          waitForNextReference = true; // Start delay before updating reference
        }
  
        // Auto-repeat check after 1 second
        else if ( millis() - lastRepeatTime >= 500 ) 
        {
          if (rawY >= referenceY + threshold) 
          {
            currentHour++;
            if (currentHour > 12) 
            {
              currentHour = 1; // Wrap around to 1 after 12
            }
            Serial.print("Auto-repeat: Hour increased to: ");
            Serial.println(currentHour);
            lastRepeatTime = millis(); // Update repeat timestamp
          } 
  
          else if (rawY <= referenceY - threshold) 
          {
            currentHour--;
            if (currentHour < 1) 
            {
              currentHour = 12; // Wrap around to 12 after 1
            }

            Serial.print("Auto-repeat: Hour decreased to: ");
            Serial.println(currentHour);
            lastRepeatTime = millis(); // Update repeat timestamp
          }
        }
        
        String mef = String( currentHour );
        mef += ":";
        if ( currentMinute < 10 )
        {
          mef += "0";
        }
        mef += String( currentMinute );

        // wfMain_SetTime_Background_Shortie

        drawImageFromFile( wfMain_SetTime_Background, true, 0, 0 );
        textmessageservice.updateTempTime( mef );
      }

      break;

    case DISPLAYING_HEALTH_STATISTICS:

      if ( needssetup )
      {
        Serial.println( "DISPLAYING_HEALTH_STATISTICS" );
        needssetup = false;
        drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
        //drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
        noMovementTime = millis();
        return;
      }

      if ( accel.tapped() )
      {
        panel = DISPLAYING_TIMER;
        video.startVideo( WatchFaceFlip2_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        needssetup = true;
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      if ( accel.doubletapped() )
      {
        panel = SETTING_HEALTH_STATISTICS;
        video.startVideo( WatchFaceFlip3_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        needssetup = true;
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }





      break;

    case SETTING_HEALTH_STATISTICS:

      if ( needssetup )
      {
        Serial.println( "SETTING_HEALTH_STATISTICS" );
        needssetup = false;
        drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
        noMovementTime = millis();
        return;
      }

      if ( accel.doubletapped() )
      {
        // Reset health stats here


        // then back to MAIN

        panel = MAIN;
        needssetup = true;
        video.startVideo( WatchFaceFlip3_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      if ( updateTimeLeft() )
      {
        panel = MAIN;
        needssetup = true;
        video.startVideo( WatchFaceFlip3_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      break;

    case DISPLAYING_TIMER:

      if ( needssetup )
      {
        Serial.println( "DISPLAYING_TIMER" );
        needssetup = false;
        drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
        noMovementTime = millis();
        return;
      }

      if ( accel.tapped() )
      {

        // Reset timer here


        panel = MAIN;
        needssetup = true;
        video.startVideo( WatchFaceFlip3_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      if ( accel.doubletapped() )
      {
        needssetup = true;
        panel = SETTING_TIMER;
        video.startVideo( WatchFaceFlip2_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      if ( updateTimeLeftNoShow() )
      {
        panel = MAIN;
        needssetup = true;
        video.startVideo( WatchFaceFlip3_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      break;

    case SETTING_TIMER:

      if ( needssetup )
      {
        Serial.println( "SETTING_TIMER" );
        needssetup = false;
        drawImageFromFile( wfMain_SetTimer_Background, true, 0, 0 );
        noMovementTime = millis();
        return;
      }

      if ( updateTimeLeft() )
      {
        // Start timer



        // and return to  Main

        panel = MAIN;
        needssetup = true;
        video.startVideo( WatchFaceFlip3_video );
        haptic.playEffect(14);  // 14 Strong Buzz
        noMovementTime = millis();
        textmessageservice.stop();
        return;
      }

      break;

  }

}
