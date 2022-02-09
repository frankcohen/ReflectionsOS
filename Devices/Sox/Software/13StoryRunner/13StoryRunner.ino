/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for story-level code, setup, and loops. Additional source:
Story ( decodes TAR to storage, set-up story elements like gestures, location triggers )
Video
Utils ( General purpose tasks )
Audio
Storage ( Gets media/data from Cloud City server, over Wifi and Bluetooth )
Gestures
Movement (IMU + GPS)
Haptic (vibrates the device)
Buttons

*/

#include "Arduino.h"
#include "AudioPlayer.h"
#include "Audio.h"
#include "Utils.h"
#include "Video.h"
#include "Storage.h"
#include "Gestures.h"
#include "Movement.h"
#include "Haptic.h"
#include "Buttons.h"
#include "MjpegClass.h"
#include "Shows.h"

AudioPlayer audioplayer;
Utils utils;
Video video;
Storage storage;
Gestures gestures;
Movement movement;
Haptic haptic;
Buttons buttons;
Shows shows;

// These are here because I don't understand C++ classes, static variables, and how to pass callbacks.
// I put the file-related functions into Video.cpp/Video.h and kept the static variables and callbacks
// needed by MjpegClass here. Anyone want to fix this?

#include <Arduino_GFX_Library.h>
Arduino_DataBus *bus = new Arduino_ESP32SPI(SPI_DisplayDC /* DC */, SPI_DisplayCS /* CS */, SPI_SCK /* SCK */, SPI_MOSI /* MOSI */, SPI_MISO /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, SPI_DisplayRST, 2 /* rotation */, false /* IPS */);
static MjpegClass mjpeg;
uint8_t *mjpeg_buf;
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame
boolean firsttime = true;
File vidfile;
int ShowCount = 0;

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long start = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  video.addTotal_show_video( millis() - start );
  return 1;
}

void setup()
{
  utils.begin();

  //Remove all files on the SD
  if ( 0 )
  {
    storage.removeFiles(SD, "/", 100 );
    storage.removeDirectories( SD, "/", 100 );
    storage.listDir(SD, "/", 100);
  }

  //audioplayer.begin();
  storage.begin();
  gestures.begin();
  movement.begin();
  haptic.begin();
  buttons.begin();
  shows.begin();

  video.begin();
  video.setReadyForNextMedia( true );

  mjpeg_buf = (uint8_t *) malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println(F("mjpeg_buf malloc failed"));
    while(1);
  }

  // Init Display
  gfx->begin();
  gfx->fillScreen(BLUE);
  gfx->invertDisplay(true);
  gfx->setRotation(0);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

long myt = millis() + 100;

void loop()
{
  // Cooperative multi-tasking functions

  if ( video.getReadyForNextMedia() )
  {
    if ( ! shows.findNext() ) return;

    Serial.print( "shows.getNextVideo(): " );
    Serial.println( shows.getNextVideo() );

    vidfile = SD.open( shows.getNextVideo(), FILE_READ );
    if ( ! vidfile )
    {
      Serial.print("Could not open: ");
      Serial.print( shows.getNextVideo() );
      return;
    }

    video.setMjpegFile( vidfile );
    video.setReadyForNextMedia( false );
    video.setNeedsSetup( true );
    video.clearNeedsPlay();

    //audioplayer.start( shows.getNextAudio() );

    Serial.print( "Show " );
    Serial.println ( ShowCount++ );
  }

  if ( video.needsSetup() )
  {
    video.clearNeedsSetup();

    File myFile = video.getMjpegFile();
    if ( ! myFile )
    {
      Serial.print( "needsSetup myFile did not open" );
      while(1);
    }

    if ( mjpeg.setup(
       &myFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
       0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */, firsttime) )
    {
      Serial.println( "Mjpeg set-up");
      firsttime = false;
    }
    else
    {
      Serial.println( "Could not set-up mjpeg");
      video.clearNeedsPlay();
      video.setReadyForNextMedia( true );
      video.clearNeedsSetup();
    }
  }

  if (  video.needsPlay() )
  {
    video.clearNeedsPlay();

    mjpeg.readMjpegBuf();
    mjpeg.drawJpg();
    smartdelay(10);
  }

  //audioplayer.loop();
  video.loop();
  storage.loop();
  gestures.loop();
  movement.loop();
  haptic.loop();
  buttons.loop();
  shows.loop();
}

// Audio system interrupt callbacks

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
  Serial.println( "Audio ended" );
  Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
