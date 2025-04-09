/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires PNGdec library https://github.com/bitbank2/PNGdec

*/

#include "WatchFace.h"

WatchFace::WatchFace(){}

void WatchFace::begin()
{
  facetime = millis();
}

void WatchFace::drawMainFace()
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfBackground;
  drawImage( mef );

  // Battery indicator

  mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += wfMainBattery;

  int batlev = battery.getBatteryLevel();
  batlev = 3;
  mef += batlev + 1;

  mef += wfMainBattery2;
  drawImagePNG( mef );

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

int WatchFaceJPEGDraw( JPEGDRAW *pDraw )
{
  gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1; // Indicate success
}

int WatchFaceJPEGDrawTransparent( JPEGDRAW *pDraw )
{
  uint16_t color;
  
  for (int32_t y = 0; y < pDraw->iHeight; y++) {
      for (int32_t x = 0; x < pDraw->iWidth; x++) {
        // Read the pixel color
        color = pDraw->pPixels[ ( y * pDraw->iWidth ) + x];

          // Skip pixels with value 0xFFE0
          if ( color != TRANSPARENT_COLOR ) {
              gfx->drawPixel(pDraw->x + x, pDraw->y + y, color); // Draw the pixel on the display
          }
      }
  }

  return 1;
}

void WatchFace::drawImage( String filename )
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

void WatchFace::drawImageTransparent( String filename )
{
  Serial.print( "WatchFace draw transparent " );
  Serial.println( filename );
  jpegFile = SD.open( filename );
  if (!jpegFile) 
  {
    Serial.print( "Failed to open file " );
    Serial.println( filename );
    return;
  }

  // Decode and draw the JPEG
  jpeg.open(jpegFile, WatchFaceJPEGDrawTransparent );
  //jpeg.open(jpegFile, WatchFaceJPEGDraw );
  jpeg.decode( 0, 0, 0 );
  
  // Close the JPEG file
  jpegFile.close();
}

File pngFile;
PNG png;  // Create a PNG decoder object

void *myOpen( const char* filename, int32_t *size )
{
  pngFile = SD.open(filename, "r");

  if (!pngFile || pngFile.isDirectory())
  {
    Serial.print( F("Failed to open " ) );
    Serial.println( filename );
  }
  else
  {
    *size = pngFile.size();
    Serial.printf("Opened '%s', size: %d\n", filename, *size);
  }

  return &pngFile;
}

void myClose(void *handle)
{
  if (pngFile)
    pngFile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!pngFile)
    return 0;
  return pngFile.read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!pngFile)
    return 0;
  return pngFile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{
  uint16_t usPixels[320];
  uint8_t usMask[320];

  // Serial.printf("Draw pos = 0,%d. size = %d x 1\n", pDraw->y, pDraw->iWidth);
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);
  gfx->draw16bitRGBBitmapWithMask(0, 0 + pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
}

void WatchFace::drawImagePNG( String filename ) 
{
    int rc;
    rc = png.open( filename.c_str(), myOpen, myClose, myRead, mySeek, PNGDraw);
    if (rc == PNG_SUCCESS)
    {
      rc = png.decode(NULL, 0);
      png.close();
    }
    else
    {
      Serial.println("png.open() failed");
    }
}

void WatchFace::loop()
{
  if ( ( millis() - facetime ) > 10000 )
  {
    facetime = millis();
    drawMainFace();
  }

}
