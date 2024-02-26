/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the video display.

*/

#include "Video.h"

// Defined in ReflectionsOfFrank.ino
extern LOGGER logger;

static MjpegClass mjpeg;
uint8_t *mjpeg_buf;

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
static Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 1 /* rotation */, false /* IPS */);
Arduino_GFX *bufgfx = new Arduino_Canvas( 240 /* width */, 240 /* height */, gfx );

#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame

/* pixel drawing callback */

static int jpegDrawCallback( JPEGDRAW *pDraw )
{
  //Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  // output_display -> flush();
  return 1;
}

Video::Video() {}

void Video::begin()
{
  #ifdef GFX_EXTRA_PRE_INIT
    GFX_EXTRA_PRE_INIT();
  #endif

  mjpeg_buf = (uint8_t *) malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println( F("mjpeg_buf malloc failed, stopping" ) );
    stopOnError( "Video buffer", "fail", "", "", "" );
  }

  gfx->begin();
  gfx->invertDisplay(true);
  gfx->fillScreen( COLOR_BACKGROUND );

  videoStatus = 0;   // idle
  firsttime = true;
  vidtimer = millis();
}

/* Show error on display, then halt */

void Video::stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 )
{
  bufgfx->begin();
  gfx->invertDisplay(true);
  gfx->fillScreen( COLOR_BACKGROUND );
  bufgfx->invertDisplay(true);
  bufgfx->fillScreen( COLOR_BACKGROUND );

  String errmsg = "Video stopOnError ";
  errmsg += msg1;
  errmsg += ", ";
  errmsg += msg2;
  errmsg += ", ";
  errmsg += msg3;
  errmsg += ", ";
  errmsg += msg4;
  errmsg += ", ";
  errmsg += msg5;
  logger.error( errmsg );

  ringtimer = millis();

  while(1)
  {
    int16_t diam = 20;
    int16_t x = random( 40, 200 );
    int16_t y = random( 40, 200 );

    bufgfx -> drawCircle( x, y, diam, COLOR_BACKGROUND);
    bufgfx -> drawCircle( x, y, diam - 1, COLOR_BACKGROUND);
    bufgfx -> drawCircle( x, y, diam - 2, COLOR_LEADING);

    bufgfx -> drawCircle( x, y, diam - 3, COLOR_RING);
    bufgfx -> drawCircle( x, y, diam - 4, COLOR_TRAILING);

    bufgfx -> fillRect( 40, 40, 160, 160, COLOR_TEXT_BACKGROUND );
    bufgfx -> drawRect( 39, 39, 162, 162, COLOR_TEXT_BORDER );
    bufgfx -> drawRect( 40, 40, 160, 160, COLOR_TEXT_BORDER );

    bufgfx->setFont(&FreeSansBold10pt7b);
    bufgfx->setTextColor( COLOR_LEADING );
    bufgfx->setCursor( leftmargin, topmargin - 5 );
    bufgfx->println("REFLECTIONS");

    bufgfx->setCursor( leftmargin, topmargin + ( 1 * linespacing ) );
    bufgfx->setFont(&FreeSerif8pt7b);
    bufgfx->setTextColor( COLOR_TEXT );
    bufgfx->println( msg1 );

    bufgfx->setCursor( leftmargin, topmargin + ( 2 * linespacing ) );
    bufgfx->setTextColor( COLOR_TEXT );
    //bufgfx->setFont(&FreeSerifBoldItalic12pt7b);
    bufgfx->println( msg2 );

    bufgfx->setCursor( leftmargin, topmargin + ( 3 * linespacing ) );
    bufgfx->println( msg3 );

    bufgfx->setCursor( leftmargin, topmargin + ( 4 * linespacing ) );
    bufgfx->println( msg4 );

    bufgfx->setCursor( leftmargin, topmargin + ( 5 * linespacing ) );
    bufgfx->println( msg5 );

    bufgfx -> flush();

    delay(500);
  }
}

void Video::addReadTime( unsigned long rtime )
{
  totalReadVideo += rtime;
}

void Video::resetStats()
{
  totalFrames = 0;
  totalReadVideo = 0;
  totalDecodeVideo = 0;
  totalShowVideo = 0; 
  startMs = millis();
}

int Video::getStatus()
{
  return videoStatus;
}

void Video::startVideo( String vname )
{
  mjpegFile = SD.open( vname );
  if ( ! mjpegFile )
  {
    videoStatus = 0;
    String msg = "Video startVideo failed to open ";
    msg += vname;
    logger.error( msg );
    return;
  }

  String msg = "mjpegFile = ";
  msg += mjpegFile.path();
  logger.info( msg );

  if ( mjpeg.setup(
    &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
    0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */, firsttime ) )
  {
    logger.info( F( "Mjpeg is set-up" ) );
    firsttime = false;
    videoStatus = 1;
  }
  else
  {
    Serial.println( F( "Could not set-up mjpeg") );
    videoStatus = 0;
  }
}

void Video::stopVideo()
{
  mjpegFile.close();
  videoStatus = 0;
}

void Video::loop()
{
  if ( videoStatus == 0 ) return;

  if ( mjpegFile.available() )
  {
    if ( (millis() - vidtimer ) > 50 ) 
    {
      vidtimer = millis();

      totalFrames++;

      unsigned long dtime = millis();

      if ( ! mjpeg.readMjpegBuf() )
      {
        logger.error( F("readMjpegBuf returned false") );
        videoStatus = 0;
        mjpegFile.close();
      }

      totalDecodeVideo += millis() - dtime;
      dtime = millis();
      mjpeg.drawJpg();
      totalShowVideo += millis() - dtime;
    }
  }
  else
  {
    stopVideo();

    totalTime = millis() - startMs;

    String stats = "Video stats, ";
    stats += "Total frames ";
    stats += totalFrames;
    stats += ", time used ";
    stats += totalTime;

    stats += ", totalReadVideo ";
    stats += totalReadVideo;
    stats += ", totalDecodeVideo ";
    stats += totalDecodeVideo;
    stats += ", totalShowVideo ";
    stats += totalShowVideo;
    
    stats += ", read ";
    stats += ( 100.0 * ( totalReadVideo / totalTime ) );
    stats += "%, decode ";
    stats += ( 100.0 * ( totalDecodeVideo / totalTime ) );
    stats += "%, show ";
    stats += ( 100.0 * ( totalShowVideo / totalTime ) );
    stats += "%, fps ";
    stats += ( 1000.0 * ( totalFrames / totalTime ) );
    logger.info( stats );
  }

}
