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
Note: ran out of memory, so no frame buffering right now.

Static JPEG images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

*/

#include "Video.h"

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 1 /* rotation */, true /* IPS */);

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
  delay(300);

  if ( battery.isBatteryLow() )
  {
    gfx->setFont(&Minya16pt7b);
    gfx->setTextSize(1);
    gfx->setCursor(45, 70);
    gfx->setTextColor(COLOR_TEXT_YELLOW);
    gfx->println(F("Battery low"));

    delay(3000);

    // Go protect RTC immediately
    hardware.prepareForSleep();
    hardware.powerDownComponents();
    esp_deep_sleep_start();
  }

  gfx->fillCircle( 120, 120, 5, COLOR_PANTONE_310 );
  delay(300);
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillCircle( 140, 120, 5, COLOR_PANTONE_102 );
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillCircle( 120, 140, 5, COLOR_PANTONE_102 );
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillCircle( 100, 120, 5, COLOR_PANTONE_102 );
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillTriangle( 115, 102, 120, 90, 125, 102, COLOR_PANTONE_151);
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);

  videoStatus = false;   // idle
  vidtimer = millis();

  curr_ms = millis();
  videoStartTime = millis();

  Serial.println( "Video started" );
}

void Video::addReadTime( unsigned long rtime )
{
  totalReadVideo += rtime;
}

/* Show error on display, then halt */

void Video::stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 )
{
  gfx->fillScreen( COLOR_BACKGROUND );

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

  gfx->fillScreen( COLOR_PANTONE_577 );
  gfx -> fillRect( 40, 40, 160, 160, COLOR_PANTONE_662 );
  gfx -> drawRect( 39, 39, 162, 162, COLOR_PANTONE_102 );
  gfx -> drawRect( 40, 40, 160, 160, COLOR_PANTONE_151 );

  gfx->setFont(&ScienceFair14pt7b);
  gfx->setTextColor( COLOR_PANTONE_102 );
  gfx->setCursor( leftmargin, topmargin - 5 );
  gfx->println(F("OH BOTHER"));

  gfx->setCursor( leftmargin, topmargin + ( 1 * linespacing ) );
  gfx->setFont(&ScienceFair14pt7b);
  gfx->setTextColor( COLOR_PANTONE_310 );
  gfx->println( msg1 );

  gfx->setCursor( leftmargin, topmargin + ( 2 * linespacing ) );
  gfx->println( msg2 );

  gfx->setCursor( leftmargin, topmargin + ( 3 * linespacing ) );
  gfx->println( msg3 );

  gfx->setCursor( leftmargin, topmargin + ( 4 * linespacing ) );
  gfx->println( msg4 );

  gfx->setCursor( leftmargin, topmargin + ( 5 * linespacing ) );
  gfx->println( msg5 );
  
  while (1)
  {
    delay(500);
  }  
}

// Returns milliseconds since the video started playing

unsigned long Video::getVideoTime()
{
  return millis() - videoStartTime;
}

void Video::displayCentered( String msg, int yq )
{
  gfx->setFont( &Minya16pt7b );
  gfx->setTextSize(1);
  gfx->getTextBounds( msg.c_str(), 0, 0, &xp, &yp, &wp, &hp);
  gfx->setCursor( (gfx->width() - wp) / 2, 85 + ( hp * yq ) );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( msg );
}

void Video::displayTextMessage( String msg, String msg2, String msg3, String msg4 )
{
  gfx->begin();
  gfx->fillScreen( COLOR_PANTONE_662 );
  displayCentered( msg, 0 );
  displayCentered( msg2, 1 );
  displayCentered( msg3, 2 );
  displayCentered( msg4, 3 );
}

void Video::addCRLF(String &s, size_t lineLen) 
{
  size_t pos = lineLen;
  while (pos < s.length()) {
    // take everything up to pos, add "\r\n", then the rest
    s = s.substring(0, pos)
      + "\r\n   "
      + s.substring(pos);
    pos += lineLen + 2;   // skip over the chunk we just processed + the CRLF
  }
}

// Paint debug info over the display

void Video::paintText( String mef )
{
  gfx->setFont(nullptr);           // NULL = default system font (5×7)
  gfx->setTextSize(2);             // leave at 1 for true 5×7
  gfx->setCursor( 30, 30 );
  gfx->setTextColor( WHITE, BLUE );
  gfx->println( mef );
}

void Video::resetStats()
{
  totalFrames = 0;
  totalReadVideo = 0;
  totalDecodeVideo = 0;
  totalShowVideo = 0; 
  startMs = millis();
}

bool Video::getStatus()
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
    videoStatus = false;
    String msg = F("startVideo failed to open ");
    msg += mef;
    logger.error( msg );
    return;
  }

  if ( mjpegrunner.start( &mjpegFile ) )
  {
    videoStatus = true;
  }
  else
  {
    Serial.println( F( "MjpegRunner did not start") );
    videoStatus = false;
    return;
  }

  videoStartTime = millis();

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
}

void Video::stopVideo()
{
  if (mjpegFile && mjpegFile.available()) 
  {
    mjpegFile.close();
  }
  setPaused( false );
  videoStatus = false;
}

void Video::setPaused( bool p )
{
  paused = p;
}

void Video::loop()
{
  if ( ( ! videoStatus ) || paused ) return;

  if ( (millis() - vidtimer ) > 50 ) 
  {
    if ( mjpegFile.available() )
    {
      vidtimer = millis();

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
    else
    {
      stopVideo();
    }
  }
}
