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
      lastUpdate(0), // Initialize lastUpdate
      animating(false) // Initialize animating state
{}

void WatchFaceMain::drawMainFace()
{
  start();

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

  // Cat face in the middle

  mef = wfMainFace;
  mef += "1";
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

/*

void WatchFaceMain::drawImage( String filename )
{
  Serial.print( "WatchFace drawImage " );
  Serial.println( filename );
  jpegFile = SD.open( filename );
  if (!jpegFile) 
  {
    Serial.print( "Failed to open file " );
    Serial.println( filename );
    return;
  }

  // Decode and draw the JPEG
  jpeg.open(jpegFile, WatchFaceJPEGDraw );
  //jpeg.open(jpegFile, WatchFaceJPEGDraw );
  jpeg.decode( 0, 0, 0 );
  
  // Close the JPEG file
  jpegFile.close();
}

*/

void WatchFaceMain::begin() 
{
  Serial.println( "WatchFaceMain begin()" );

  WatchFaceBase::begin();  // This ensures the base method is executed

  facetime = millis();

  currentHour = 0;
  currentMinute = 0;
  targetHour = 0;
  targetMinute = 0;

  animating = true;
}

void WatchFaceMain::loop()
{
  if ( ( millis() - facetime ) > 10000 )
  {
    facetime = millis();
    drawMainFace();
  }

/*
    // Hours and minutes animate clockwise and counterclockwist into position

    uint32_t currentTime = millis();
    if (!animating) {
        // Start animation to current hour and minute
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            targetHour = timeinfo.tm_hour % 12; // Convert to 12-hour format
            targetMinute = timeinfo.tm_min / 3;  // Convert to 1 of 20 positions around dial
            animationStartTime = currentTime;
            animating = true;
        }
    } else {
        // Animate hours clockwise and minutes counterclockwise
        uint32_t elapsed = currentTime - animationStartTime;
        float progress = min(1.0f, elapsed / 2000.0f); // Animation lasts up to 2 seconds

        int newHour = currentHour + (int)((targetHour - currentHour) * progress);
        int newMinute = currentMinute + (int)((targetMinute - currentMinute) * -progress);
*/

/*
        // Clear the canvas
        bufferCanvas->fillScreen(0);

        // Draw hour image
        char hourFilename[32];
        snprintf(hourFilename, sizeof(hourFilename), "Hours_%d.png", newHour + 1);
        drawPngFromFile(hourFilename, 0, 0);

        // Draw minute image
        char minuteFilename[32];
        snprintf(minuteFilename, sizeof(minuteFilename), "Minutes_%d.png", newMinute + 1);
        drawPngFromFile(minuteFilename, 0, 0);

        gfx->drawRGBBitmap(0, 0, bufferCanvas->getBuffer(), 240, 240);
*/

/*
        // End animation if complete
        if (progress >= 1.0f) {
            currentHour = targetHour;
            currentMinute = targetMinute;
            animating = false;
        }


    }  
*/

}
