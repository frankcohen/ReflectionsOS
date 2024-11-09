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

*/

#include "WatchFaceBase.h"

// Optimized JPEGDEC callback function to draw raw byte data onto the canvas buffer

static int WatchFaceJPEGDraw(JPEGDRAW *pDraw)
{
  bufferCanvas->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}

/* Draw a PNG with transparent background */

File pngFile;
PNG png;  // Create a PNG decoder object
JPEGDEC jpeg;

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

  if ( bufferCanvas ) 
  {
    bufferCanvas->draw16bitRGBBitmapWithMask(0, pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
  }
}

WatchFaceBase::WatchFaceBase() 
{}

void WatchFaceBase::start() {
  if (bufferCanvas) {
    //bufferCanvas->invertDisplay( true );
    //bufferCanvas->fillScreen(BLACK); // Clear the buffer to prepare for new drawings
  } else {
      Serial.println("Error: bufferCanvas is not initialized.");
  }
}

void WatchFaceBase::show() 
{
  bufferCanvas->invertDisplay( true );
  bufferCanvas->flush();
}

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
    if (!file) {
        Serial.printf("Failed to open %s\n", filename);
        return;
    }

    if ( filename.endsWith( ".jpg"))
    {
      // Decode and draw the JPEG
      jpeg.open( file, WatchFaceJPEGDraw );
      jpeg.decode( 0, 0, 0 );      
    }

    if ( filename.endsWith( ".png"))
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

    file.close();
}

/*
    uint8_t result = png.open(file, 0);
    if (result == PNG_SUCCESS) {
        bufferCanvas->startWrite();
        png.decode([](PNGDRAW *pDraw) {
            uint16_t usPixels[320];
            uint8_t usMask[320];

            png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
            png.getAlphaMask(pDraw, usMask, 1);
            gfx->draw16bitRGBBitmapWithMask(pDraw->x, pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
        });
        bufferCanvas->endWrite();
        gfx->drawRGBBitmap(0, 0, bufferCanvas->getBuffer(), 240, 240);
        png.close();
    } else {
        Serial.printf("PNG open failed: %d\n", result);
    }
*/

/*
void WatchFaceBase::drawJpegFromFile(const char *filename, int16_t x, int16_t y) 
{
    String mef = "/";
    mef += NAND_BASE_DIR;
    mef += "/";
    mef += filename;
    filename = mef.c_str();

    File file = SD.open(filename);
    if (!file) {
        Serial.printf("Failed to open %s\n", filename);
        return;
    }

    if (jpeg.open(file, JPEGDEC::JPG_SCALE_NONE) == 0) {
        bufferCanvas->startWrite();
        jpeg.decode([](JDEC *jdec, void *bitmap, JRECT *rect) {
            uint16_t *pPixels = (uint16_t *)bitmap;
            for (int16_t py = rect->top; py <= rect->bottom; py++) {
                for (int16_t px = rect->left; px <= rect->right; px++) {
                    gfx->writePixel(px, py, *pPixels++);
                }
            }
        });
        bufferCanvas->endWrite();
        gfx->drawRGBBitmap(0, 0, bufferCanvas->getBuffer(), 240, 240);
        jpeg.close();
    } else {
        Serial.printf("JPEG open failed\n");
    }

    file.close();
}
*/

/* To draw a JPEG with a transparent color use this */

/*
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
*/

void WatchFaceBase::begin() 
{
}

void WatchFaceBase::loop() {
    // Default implementation can be empty or have repetitive logic.
}
