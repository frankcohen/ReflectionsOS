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

void WatchFaceMain::drawMainFace()
{
  start();

/*
  drawImageFromFile( wfMainBackground, true, 0, 0 );
  
  // Battery indicator

  mef = wfMainBattery;
  int batlev = battery.getBatteryLevel();
  batlev = 3;
  mef += batlev + 1;

  mef += wfMainBattery2;

  drawImageFromFile( mef, true, 0, 0 );

  // Hours

  mef = wfMainHours;

  // Get current time
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);

  // Get the hour in 12-hour format
  int hour12 = timeinfo->tm_hour % 12;
  if (hour12 == 0) {
      hour12 = 12; // Convert 0 to 12 for 12-hour format
  }

  Serial.print( "Time hours: " );
  Serial.println( hour12 );

  mef += hour12;

  mef += wfMainHours2;
  drawImageFromFile( mef, true, 0, 0 );

  // Minutes

  mef = wfMainMinutes;

  int currentMinute = timeinfo->tm_min;
  int imageIndex = currentMinute % 21; // This will loop back to 0 after 20

  mef += imageIndex;
  mef += wfMainMinutes2;

  Serial.print( "Time minutes: " );
  Serial.print( currentMinute );
  Serial.print( ", " );
  Serial.println( imageIndex );

  drawImageFromFile( mef, true, 0, 0 );
*/

  // Cat face in the middle

  mef = wfMainFace;
  mef += "2";
  mef += wfMainFace2;
  drawImageFromFile( mef, true, 0, 0 );

  show();
}

/*
void WatchFaceMain::drawMainFace()
{
  drawJpegFromFile( wfMainBackground, 0, 0 );

  // Battery indicator

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainBattery;

  int batlev = battery.getBatteryLevel();
  batlev = 3;
  mef += batlev + 1;

  mef += wfMainBattery2;

  drawPngFromFile( mef, 0, 0 )

  // Hours

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainHours;

  // Get current time
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);

  // Get the hour in 12-hour format
  int hour12 = timeinfo->tm_hour % 12;
  if (hour12 == 0) {
      hour12 = 12; // Convert 0 to 12 for 12-hour format
  }

  Serial.print( "Time hours: " );
  Serial.println( hour12 );

  mef += hour12;

  mef += wfMainHours2;
  drawImagePNG( mef );

  // Minutes

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainMinutes;

  int currentMinute = timeinfo->tm_min;
  int imageIndex = currentMinute % 21; // This will loop back to 0 after 20

  mef += imageIndex;
  mef += wfMainMinutes2;

  Serial.print( "Time minutes: " );
  Serial.print( currentMinute );
  Serial.print( ", " );
  Serial.println( imageIndex );

  drawImagePNG( mef );

  // Cat face in the middle

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainFace;
  mef += "1";
  mef += wfMainFace2;
  drawImagePNG( mef );

  delay(blinkspeed);

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainFace;
  mef += "2";
  mef += wfMainFace2;
  drawImagePNG( mef );

  delay(blinkspeed);

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainFace;
  mef += "3";
  mef += wfMainFace2;
  drawImagePNG( mef );
  delay(blinkspeed);

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainFace;
  mef += "4";
  mef += wfMainFace2;
  drawImagePNG( mef );

  delay( blinkspeed * 2);

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainFace;
  mef += "3";
  mef += wfMainFace2;
  drawImagePNG( mef );

  delay(blinkspeed);

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainFace;
  mef += "2";
  mef += wfMainFace2;
  drawImagePNG( mef );
  delay(blinkspeed);

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfBackground;
  drawImage( mef );

  for ( int i = 1; i < 7; i++ )
  {
    mef = "/";
    mef += NAND_BASE_DIR;
    mef += "/";
    mef += wfMainFlip;
    mef += i;
    mef += wfMainFlip2;
    drawImagePNG( mef );
    delay( flipspeed );
  }

  timeservice.startShow( 1 );   // Shows digital time
  timeservice.setTimeAnimationActivated( true );
}

*/

void WatchFaceMain::begin() 
{
  WatchFaceBase::begin();  // This ensures the base method is executed

  facetime = millis();
  hoursmintimer = millis();
  startupComplete = false;
}

/* Hours and minutes animate clockwise and counterclockwist into position */

void WatchFaceMain::beginStartupAnimation()
{
  struct tm timeinfo;

  if ( getLocalTime( &timeinfo ) ) 
  {
    startHour = timeinfo.tm_hour % 12; // Convert to 12-hour format
    startMinute = timeinfo.tm_min / 3;  // Convert to 1 of 20 positions around dial
  }
  else
  {
    startHour = 2;
    startMinute = 15;
  }
  
  counter = 0;

  startupComplete = false;

  battimer = millis();
  batcount = 0;
  batlev = 0;

}

bool WatchFaceMain::startupAnimation()
{
  if ( ( millis() - hoursmintimer ) < 100 ) return false;
  hoursmintimer = millis();

  // Normalize the startHour (1 to 12) and startMinute (1 to 20)
  startHour = (startHour % 12 == 0) ? 12 : startHour;
  startMinute = (startMinute % 20 == 0) ? 20 : startMinute;

  // Calculate the current minute position in counterclockwise direction
  currentMinute = (startMinute - counter - 1 + 20) % 20 + 1;

  // Calculate the relative hour position based on minute progress
  float hourProgressFloat = (counter * 12.0) / 20.0;  // The hour hand moves 12 positions in 20 minute steps
  int hourProgress = (int)ceil(hourProgressFloat);
  currentHour = (startHour + hourProgress - 1) % 12 + 1;

  // Increment the counter to move to the next position
  counter++;

  // If the counter has reached 20, it means we've gone through a full cycle
  if (counter >= 20) 
  {
    counter = 0;  // Reset counter for next cycle
    return true;  // Full cycle completed
  }

  // Draw the elements to the buffer

  start();    // Clear frame buffer

  drawImageFromFile( wfMainBackground, true, 0, 0 );

  // Draw hour image
  String mef = wfMainHours;
  mef += currentHour;
  mef += wfMainHours2;
  drawImageFromFile( mef, true, 0, 0 );

  // Draw minutes image
  mef = wfMainMinutes;
  mef += currentMinute;
  mef += wfMainMinutes2;
  drawImageFromFile( mef, true, 0, 0 );

  // Battery indicator

  if ( ( millis() - battimer ) > 2000 )
  {
    battimer = millis();

    batlev = battery.getBatteryLevel();
    if ( ( batlev != batcount ) && ( batcount < 4 ) ) batcount++;
  }

  mef = wfMainBattery;
  mef += batcount + 1;
  mef += wfMainBattery2;
  drawImageFromFile( mef, true, 0, 0 );

  show();

  /*
  Serial.print( counter );
  Serial.print(", Start: ");
  Serial.print( startHour);
  Serial.print(":" );
  Serial.print( startMinute);
  Serial.print(" Current: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print( " Battery " );
  Serial.print( batlev );
  Serial.print( ":" );
  Serial.println( batcount );
  */
  
  // Return false if there's more to go
  return false;
}

void WatchFaceMain::loop()
{


  // Restructure this code:
  //   Start everything all at once
  //   Update the watch face ( hands animate, battery leaves bloom, cat face blinks and smiles, then push)
  //   Add interrupts for time, alarm
  
  // Then in WatchFaceExperiences, add the wrist and tof gestures

  // Set time
  // Clear health stats
  // Set alarm






  if ( ( millis() - facetime ) > 20000 )
  {
    facetime = millis();
    //drawMainFace();

    // Blink eyes

    // Smile

    // Yawn

    // Sleep processor, keep display on

    beginStartupAnimation();
  }

  if ( ! startupComplete )
  {
    startupComplete = startupAnimation();
  }

}
