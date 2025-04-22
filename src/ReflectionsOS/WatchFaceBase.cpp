/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires PNGdec library https://github.com/bitbank2/PNGdec

 Uses framebuffer capability in Arduino_GFX. Tutorial is at
 https://github.com/moononournation/Arduino_GFX/wiki/Canvas-Class

Images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

This is the way to draw JPEG images with transparent pixels. However, it operates
much slower than the above draw16bitBeRGBBitmap() method.

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

*/

#include "WatchFaceBase.h"

/* Static variables and callback functions */

File imgFile;
PNG png;  // Create a PNG decoder object

//
// Create a private structure to pass info to the draw callback
// For this example we want to pass a random x/y starting point
//
typedef struct my_private_struct
{
  int xoff, yoff; // corner offset
} PRIVATE;

// Optimized JPEGDEC callback function to draw raw byte data onto the canvas buffer

static int WatchFaceJPEGDraw(JPEGDRAW *pDraw)
{
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);

  /* This is the way to draw JPEG images with transparent pixels. However, it operates
     much slower than the above draw16bitBeRGBBitmap() method.

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
  */

  return 1;
}

// Function to draw pixels to the display
void WatchFacePNGDraw(PNGDRAW *pDraw)
{
  uint16_t usPixels[320];
  uint8_t usMask[320];

  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);
  gfx->draw16bitRGBBitmapWithMask(0, pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
}

void *myOpen( const char* filename, int32_t *size )
{
  imgFile = SD.open(filename, "r");

  if (!imgFile || imgFile.isDirectory())
  {
    Serial.print( F("Failed to open " ) );
    Serial.println( filename );
  }
  else
  {
    *size = imgFile.size();
    //Serial.printf("Opened '%s', size: %d\n", filename, *size);
  }

  return &imgFile;
}

void myClose(void *handle)
{
  if ( imgFile )
    imgFile.close();
}

int32_t myReadJpeg( JPEGFILE *pFile, uint8_t *buffer, int32_t length )
{
  if (!imgFile)
    return 0;
  return imgFile.read(buffer, length);
}

int32_t myReadPng( PNGFILE *handle, uint8_t *buffer, int32_t length )
{
  if (!imgFile)
    return 0;
  return imgFile.read(buffer, length);
}

int32_t mySeekJpeg( JPEGFILE *pFile, int32_t position)
{
  if (!imgFile)
    return 0;
  return imgFile.seek(position);
}

int32_t mySeekPng(PNGFILE *handle, int32_t position)
{
  if (!imgFile)
    return 0;
  return imgFile.seek(position);
}

bool WatchFaceBase::isRunning()
{
  return _runmode;
}

void WatchFaceBase::setRunning( bool _run )
{
  _runmode = _run;
}

WatchFaceBase::WatchFaceBase() 
{}

void WatchFaceBase::start() 
{}

void WatchFaceBase::show() 
{}

/* Uses file extension for image type, .png, .jpg */

void WatchFaceBase::drawImageFromFile( String filename, bool embellishfilename, int16_t x, int16_t y) 
{
  if ( embellishfilename )
  {
    String mef = "/";
    mef += NAND_BASE_DIR;
    mef += "/";
    mef += filename;
    filename = mef.c_str();
  }  

  File file = SD.open( filename );
  if (!file) 
  {
    Serial.printf("Failed to open %s\n", filename);
    return;
  }

  if ( filename.endsWith( F(".jpg")))
  {
    jpg.open(filename.c_str(), myOpen, myClose, myReadJpeg, mySeekJpeg, WatchFaceJPEGDraw);

    jpg.setPixelType( RGB565_BIG_ENDIAN );

    // Decode and draw the JPEG
    jpg.decode( 0, 0, 0 );   // x, y, scale  
    file.close();
  }

  if ( filename.endsWith( ".png"))
  {
    int rc;
    rc = png.open( filename.c_str(), myOpen, myClose, myReadPng, mySeekPng, WatchFacePNGDraw);
    if (rc == PNG_SUCCESS)
    {
      PRIVATE priv;
      priv.xoff = 0;
      priv.yoff = 0;
      rc = png.decode( (void *) &priv, 0);
      png.close();
    }
    else
    {
      Serial.println(F("png.open() failed"));
    }
  }
}

void WatchFaceBase::begin() 
{
}

void WatchFaceBase::loop() 
{
}
