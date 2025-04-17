/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for interacting with the video display.

Static JPEG images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

Some classes use the framebuffer capability in Arduino_GFX. Tutorial is at
https://github.com/moononournation/Arduino_GFX/wiki/Canvas-Class

Images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

*/

#include "Video.h"

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 1 /* rotation */, true /* IPS */);
Arduino_Canvas *bufferCanvas = new Arduino_Canvas(240, 240, gfx);

Video::Video() {}

void Video::begin()
{
  #ifdef GFX_EXTRA_PRE_INIT
    GFX_EXTRA_PRE_INIT();
  #endif

  if ( ! gfx->begin() )
  {
    Serial.println(F("gfx->begin() failed. Stopping."));
    while(1);
  }

  gfx->fillScreen( BLACK );

  videoStatus = 0;   // idle
  vidtimer = millis();
  paused = false;

  curr_ms = millis();
  videoStartTime = millis();
}

// Initializes video buffer

void Video::beginBuffer()
{
  if ( ! bufferCanvas->begin() )
  {
    Serial.println( F( "bufferCanvas->begin() failed. Stopping." ) );
    while(1);
  }
  else
  {
    //Serial.println( F( "bufferCanvas->begin() suceeded" ) );
  }
  bufferCanvas->invertDisplay(true);
  bufferCanvas->fillScreen( BLUE );
}

void Video::addReadTime( unsigned long rtime )
{
  totalReadVideo += rtime;
}

/* Show error on display, then halt */

void Video::stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 )
{
  bufferCanvas->begin();
  bufferCanvas->fillScreen( COLOR_BACKGROUND );

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  String errmsg = F("Video stopOnError ");
  errmsg += msg1;
  errmsg += F(", ");
  errmsg += msg2;
  errmsg += F(", ");
  errmsg += msg3;
  errmsg += F(", ");
  errmsg += msg4;
  errmsg += F(", ");
  errmsg += msg5;
  logger.error( errmsg );

  ringtimer = millis();

  while(1)
  {
    int16_t diam = 20;
    int16_t x = random( 40, 200 );
    int16_t y = random( 40, 200 );

    bufferCanvas -> drawCircle( x, y, diam, COLOR_BACKGROUND);
    bufferCanvas -> drawCircle( x, y, diam - 1, COLOR_BACKGROUND);
    bufferCanvas -> drawCircle( x, y, diam - 2, COLOR_LEADING);

    bufferCanvas -> drawCircle( x, y, diam - 3, COLOR_RING);
    bufferCanvas -> drawCircle( x, y, diam - 4, COLOR_TRAILING);

    bufferCanvas -> fillRect( 40, 40, 160, 160, COLOR_TEXT_BACKGROUND );
    bufferCanvas -> drawRect( 39, 39, 162, 162, COLOR_TEXT_BORDER );
    bufferCanvas -> drawRect( 40, 40, 160, 160, COLOR_TEXT_BORDER );

    bufferCanvas->setFont(&ScienceFair14pt7b);
    bufferCanvas->setTextColor( COLOR_LEADING );
    bufferCanvas->setCursor( leftmargin, topmargin - 5 );
    bufferCanvas->println(F("REFLECTIONS"));

    bufferCanvas->setCursor( leftmargin, topmargin + ( 1 * linespacing ) );
    bufferCanvas->setFont(&ScienceFair14pt7b);
    bufferCanvas->setTextColor( COLOR_TEXT );
    bufferCanvas->println( msg1 );

    bufferCanvas->setCursor( leftmargin, topmargin + ( 2 * linespacing ) );
    bufferCanvas->setTextColor( COLOR_TEXT );
    bufferCanvas->println( msg2 );

    bufferCanvas->setCursor( leftmargin, topmargin + ( 3 * linespacing ) );
    bufferCanvas->println( msg3 );

    bufferCanvas->setCursor( leftmargin, topmargin + ( 4 * linespacing ) );
    bufferCanvas->println( msg4 );

    bufferCanvas->setCursor( leftmargin, topmargin + ( 5 * linespacing ) );
    bufferCanvas->println( msg5 );

    bufferCanvas -> flush();

    delay(500);
  }
}

// Returns milliseconds since the video started playing

unsigned long Video::getVideoTime()
{
  return millis() - videoStartTime;
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

/* Play a .mjpeg file to the display */

void Video::startVideo( String vname )
{
  String mef = F("/");
  mef += NAND_BASE_DIR;
  mef += F("/");
  mef += vname;
  mef += F("/");
  mef += vname;
  mef += videoname_end;

  String msg = F("startVideo ");
  msg += mef;
  logger.info( msg );
  
  mjpegFile = SD.open( mef );
  if ( ! mjpegFile )
  {
    videoStatus = 0;
    String msg = F("startVideo failed to open ");
    msg += mef;
    logger.error( msg );
    return;
  }

  if ( mjpegrunner.start( &mjpegFile ) )
  {
    videoStatus = 1;
  }
  else
  {
    Serial.println( F( "MjpegRunner did not start") );
    videoStatus = 0;
    return;
  }

  videoStartTime = millis();

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
}

void Video::stopVideo()
{
  mjpegFile.close();
  videoStatus = 0;
}

void Video::setPaused( bool p )
{
  paused = p;
}

void Video::loop()
{
  if ( ( videoStatus == 0 ) || ( paused == 1 ) ) return;

  if ( mjpegFile.available() )
  {
    if ( (millis() - vidtimer ) > 50 ) 
    {
      vidtimer = millis();

      totalFrames++;

      unsigned long dtime = millis();

      if ( ! mjpegrunner.readMjpegBuf() )
      {
        stopVideo();
        return;
      }

      totalFrames++;

      mjpegrunner.drawJpg();

      totalShowVideo += millis() - dtime;
    }
  }
}
