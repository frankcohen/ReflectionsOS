/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires PNGdec library https://github.com/bitbank2/PNGdec

*/

#include "WatchFaceMain.h"

WatchFaceMain::WatchFaceMain() 
    : WatchFaceBase(), // Call to the base class constructor
      currentHour(0), 
      currentMinute(0), 
      lastUpdate(0)
{}

void WatchFaceMain::begin() 
{
  WatchFaceBase::begin();  // This ensures the base method is executed

  facetime = millis();
  hoursmintimer = millis();

  beginStartupAnimation();

  catFaceIndex = 1;
  catFaceTimer = millis();
  catFaceDirection = true;
  blinking = false;
  catFaceWait = rand() % 30000;

  displayUpdateable = true;
}

/* Hours and minutes animate clockwise and counterclockwist into position */

void WatchFaceMain::beginStartupAnimation()
{
  struct tm timeinfo;
  startHour = realtimeclock.getCurrentHourFromRTC() % 12;
  if ( startHour == -1 ) startHour = 2;

  startMinute = realtimeclock.getCurrentMinuteFromRTC() % 20;
  if ( startMinute == -1 ) startMinute = 15;
  
  counter = 0;

  startupComplete = false;

  battimer = millis();
  batcount = 0;
  batlev = 0;

  startHour = 0;
  startMinute = 0;
  currentHour = 0;
  currentMinute = 0;
  honce = true;
  monce = true;

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
}

bool WatchFaceMain::startupAnimation()
{
  // Hours and Minutes hands revolve around the cat face

  // Normalize the startHour (1 to 12) and startMinute (1 to 20)
  startHour = (startHour % 12 == 0) ? 12 : startHour;
  startMinute = (startMinute % 20 == 0) ? 20 : startMinute;

  // Calculate the current minute position in counterclockwise direction
  int currentMinute2 = (startMinute - counter - 1 + 20) % 20 + 1;
  if ( currentMinute != currentMinute2 ) displayUpdateable = true;
  currentMinute = currentMinute2;

  // Calculate the relative hour position based on minute progress
  float hourProgressFloat = (counter * 12.0) / 20.0;  // The hour hand moves 12 positions in 20 minute steps
  int hourProgress = (int)ceil(hourProgressFloat);
  int currentHour2 = (startHour + hourProgress - 1) % 12 + 1;

  if ( currentHour != currentHour2 ) displayUpdateable = true;
  currentHour = currentHour2;

  // If the counter has reached 20, it means we've gone through a full cycle
  counter++;
  if (counter >= 20) 
  {
    startupComplete = true;
    return true;
  }

  return false;
}

// Battery indicator grows/loses leaves

void WatchFaceMain::batteryMove()
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

void WatchFaceMain::timelyHoursAndMinutes()
{
  int currentHour2 = realtimeclock.getCurrentHourFromRTC();

  if ( currentHour2 == -1 ) currentHour2 = 2;

  currentHour2 = currentHour2 % 12;
  
  if ( currentHour2 != currentHour || honce )
  {
    currentHour = currentHour2;
    displayUpdateable = true;
    honce = false;
  }

  int currentMinute2 = realtimeclock.getCurrentMinuteFromRTC();
  if ( currentMinute2 == -1 ) currentMinute2 = 15;

  currentMinute2 = currentMinute2 % 20;

  if ( currentMinute2 != currentMinute || monce )
  {
    currentMinute = currentMinute2;
    displayUpdateable = true;
    monce = false;
  }

}

// Cat blinks eyes

void WatchFaceMain::blinks()
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

        Serial.print( "catFaceWait ");
        Serial.println( catFaceWait );
      }
    }
  }
}

// Draw the elements to the buffer

void WatchFaceMain::updateDisplay()
{
  if ( ! displayUpdateable ) return;
  displayUpdateable = false;

  start();    // Clear frame buffer

  // Background

  drawImageFromFile( wfMainBackground, true, 0, 0 );

  String mef;

  if ( startupComplete )
  {
    // Draw hour image
    if ( currentHour < 13 )
    {
      mef = wfMainHours;
      mef += currentHour;
      mef += wfMainHours2;
      drawImageFromFile( mef, true, 0, 0 );
    }
    else
    {
      Serial.print( "currentHour: " );
      Serial.println( currentHour );
    }

    // Draw minutes image
    if ( currentMinute < 21 )
    {
      mef = wfMainMinutes;
      mef += currentMinute;
      mef += wfMainMinutes2;
      drawImageFromFile( mef, true, 0, 0 );
    }
    else
    {
      Serial.print( "currentMinute: " );
      Serial.println( currentMinute );
    }


  }

  // Battery indicator

  mef = wfMainBattery;
  mef += batcount + 1;
  mef += wfMainBattery2;
  drawImageFromFile( mef, true, 0, 0 );

  // Cat face in the middle, and he blinks randomly

  mef = wfMainFace;
  mef += catFaceIndex;
  mef += wfMainFace2;
  drawImageFromFile( mef, true, 0, 0 );

  show();
}

void WatchFaceMain::loop()
{  
  if ( ( millis() - facetime ) > 20000 )
  {
    facetime = millis();
  }

  unsigned long mytime = millis();

  if ( ! startupComplete )
  {
    startupComplete = startupAnimation();
  }
  else
  {
    blinks();     // Blink eyes
    batteryMove();    // Battery level indicator
    timelyHoursAndMinutes();
  }

  mytime = millis();

  updateDisplay();
}
