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
Utils ( General purpose tasks )
Audio
Video
Storage ( Gets media/data from server, over Wifi )

Depends on:
JPEGDEC:     https://github.com/bitbank2/JPEGDEC.git
ArduinoJSON: https://arduinojson.org/
ESP32-targz: https://github.com/tobozo/ESP32-targz
Arduino_GFX: https://github.com/moononournation/Arduino_GFX
esp32FOTA:   https://github.com/chrisjoyce911/esp32FOTA

*/

#include "Arduino.h"
#include "Utils.h"
#include "config.h"
#include "Storage.h"
#include "Shows.h"
#include "Video.h"
#include "MjpegClass.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

Utils *utils = new Utils();
Storage *storage = new Storage();
Shows *shows = new Shows();
Video *video = new Video();

// fcohen: These are here because I don't understand C++ classes, static variables, and how to pass callbacks.
// I put the file-related functions into Video.cpp/Video.h and kept the static variables and callbacks
// needed by MjpegClass here. Anyone want to fix this?

#include <Arduino_GFX_Library.h>

static Arduino_DataBus *bus = new Arduino_HWSPI(SPI_DisplayDC, SPI_DisplayCS, SPI_SCK, SPI_MOSI, SPI_MISO);
static Arduino_GFX *gfx = new Arduino_GC9A01(bus, SPI_DisplayRST, 2 /* rotation */, false /* IPS */);

static MjpegClass mjpeg;
uint8_t *mjpeg_buf;
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame
boolean firsttime = true;
File vidfile;
int ShowCount = 0;

int xx = 0;

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  //Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long start = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  video->addTotal_show_video( millis() - start );
  return 1;
}

void setup()
{
  Serial.begin(115200);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.setDebugOutput(true);
  Serial.println("Hoober research");
    
  utils->begin();   // Turns off Wifi

  gfx->begin();
  gfx->fillScreen( BLUE );
  gfx->invertDisplay(true);

  delay(500);
  
  utils->startSDWifi(); // Start SD storage

  // OTA Over The Air update settings
  
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
   
  storage->begin();
  shows->begin();
  video->begin();
  video->setReadyForNextMedia( true );

  mjpeg_buf = (uint8_t *) malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println(F("mjpeg_buf malloc failed"));
    while(1);
  }

/*
  storage->removeFiles( SD, "/", 100 );
  storage->removeDirectories( SD, "/", 100 );
  storage->listDir( SD, "/", 100 );
*/
  
  Serial.println("Setup done");
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

void loop()
{
  // Cooperative multi-tasking functions

  ArduinoOTA.handle();

  if ( video->getReadyForNextMedia() )
  {
    if ( ! shows->findNext() ) return;

    String nv = shows->getNextVideo();
    Serial.print( "shows->getNextVideo(): " );
    Serial.println( nv );

    vidfile = SD.open( shows->getNextVideo(), FILE_READ );
    if ( ! vidfile )
    {
      Serial.print("Could not open: ");
      Serial.println( shows->getNextVideo() );
      return;
    }

    video->setMjpegFile( vidfile );
    video->setReadyForNextMedia( false );
    video->setNeedsSetup( true );
    video->clearNeedsPlay();

    Serial.print( "Show " );
    Serial.println ( ShowCount++ );
  }

  if ( video->needsSetup() )
  {
    video->clearNeedsSetup();

    File myFile = video->getMjpegFile();
    if ( ! myFile )
    {
      Serial.print( "needsSetup myFile did not open" );
      while(1);
    }
    else
    {
      Serial.print("myFile = " );
      Serial.println( myFile.path() );
    }

    if ( mjpeg.setup(
       &myFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
       0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */, firsttime ) )
    {
      Serial.println( "Mjpeg set-up");
      firsttime = false;
    }
    else
    {
      Serial.println( "Could not set-up mjpeg");
      video->clearNeedsPlay();
      video->setReadyForNextMedia( true );
      video->clearNeedsSetup();
    }
  }

  if (  video->needsPlay() )
  {
    video->clearNeedsPlay();

    mjpeg.readMjpegBuf();
    mjpeg.drawJpg();
    smartdelay(100);
  }

  video->loop();
  storage->loop();
  shows->loop();
  utils->loop();
}
