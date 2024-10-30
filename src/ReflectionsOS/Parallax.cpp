/*
 Reflections, mobile connected entertainment device

 3D wallpaper with parallax effect
 Move the device, accelerometer senses position, changes wallpaper

 Requires 6 jpg images to be named /REFLECTIONS/cat1_parallax_baseline.jpg, 2, 3, etc.
 Images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
 JPEG created in Pixelmator on Mac OS to baseline encoding using:
 ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

 Uses the X axis of the Reflections board accellerometer to determine the image.
 Future expansion should incorporate the Y and Z axis.

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Parallax.h"

Parallax::Parallax(){}

void Parallax::begin()
{   
  //gfx->begin();
  //gfx->fillScreen( BLUE );
  gfx->invertDisplay(true);

  ParallaxWaitTime = millis();

  pictureNum = 6;
}

int JPEGDraw(JPEGDRAW *pDraw)
{
  gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}

// Load JPG into memory buffer

uint8_t* Parallax::loadFileToBuffer( String filePath )
{
  String myl = F( "Parallax loading file " );
  myl += String( filePath );
  logger.info( myl );

  File file = SD.open(filePath, FILE_READ);
  if ( !file ) 
  {
    logger.error( F( "Failed to open file" ) );
    return nullptr;
  }

  // Get the size of the file
  fileSize = file.size();
  //Serial.print("File size: ");
  //Serial.println(fileSize);

  // Allocate memory for the buffer
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if ( buffer == nullptr )
  {
    logger.error( F( "Parallax failed to allocate memory" ) );
    file.close();
    return nullptr;
  }

  // Read the file into the buffer

  size_t bytesRead = file.read(buffer, fileSize);
  if ( bytesRead != fileSize )
  {
    String myl = F( "Failed to read complete file " );
    myl += String( bytesRead );
    logger.info( myl );

    free(buffer);
    file.close();
    return nullptr;
  }
  //Serial.print( "Bytes read " );
  //Serial.println( bytesRead );

  // Close the file
  file.close();
  
  // Return the buffer
  return buffer;
}

void Parallax::loop()
{
  if ( ( millis() - ParallaxWaitTime ) > 1000 )
  {
    ParallaxWaitTime = millis();

    float nx = accel.getXreading() + 1;
    if ( nx < accxleft ) nx = accxleft;
    if ( nx > accxright ) nx = accxright;
    int pose = ( (int) ( ( nx - accxleft ) / stepvalue ) ) + 1;

    int pose2 = pose;
    if ( pose == 1 ) pose2 = 6;
    if ( pose == 2 ) pose2 = 5;
    if ( pose == 3 ) pose2 = 4;
    if ( pose == 4 ) pose2 = 3;
    if ( pose == 5 ) pose2 = 2;
    if ( pose == 6 ) pose2 = 1;

    String mef = "/";
    mef += NAND_BASE_DIR;
    mef += "/";
    mef += "cat";
    mef += String( pose2 );
    mef += "_parallax_baseline.jpg";

    long lTime;

    uint8_t* mybuf = loadFileToBuffer( mef );

    if ( mybuf != nullptr )
    {
      if ( jpeg.openRAM( mybuf, fileSize, JPEGDraw ) )
      {
        lTime = micros();
        
        if ( ! jpeg.decode( 0, 0, 0 ) )
        {
          String myl = F( "Parallax decode error " );
          myl += String( jpeg.getLastError() );
          logger.info( myl );
        }

        lTime = micros() - lTime;
        // Serial.print( jpeg.getWidth() );
        // Serial.print( "x ");
        // Serial.print( jpeg.getHeight() );
        // Serial.print( "y ");
        // Serial.print( lTime );
        // Serial.println( "ms");
      }
      else
      {
        logger.error( F( "jpeg.open failed" ) );
      }
      
      free( mybuf );
    }
  }
}
