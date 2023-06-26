/*
Reflections, distributed entertainment platform

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This sketch tests all the comnponents of the Reflections board

Depends on:
JPEGDEC:     https://github.com/bitbank2/JPEGDEC.git
ArduinoJSON: https://arduinojson.org/
ESP32-targz: https://github.com/tobozo/ESP32-targz
Arduino_GFX: https://github.com/moononournation/Arduino_GFX
esp32FOTA:   https://github.com/chrisjoyce911/esp32FOTA

*/

#include "Arduino.h"
#include "BLE.h"
#include "Utils.h"
#include "config.h"
#include "Storage.h"
#include "Shows.h"
#include "Video.h"
#include "MjpegClass.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"
#include "Accellerometer.h"
#include "Audio.h"
#include "Compass.h"
#include "GPS.h"
#include "Gesture.h"
#include "Haptic.h"
#include "LED.h"
#include "USBFlashDrive.h"

static Utils utils;

// add https server capability to wwifi, from elekshack

static Storage storage;
static Shows shows;
static Video video;
static Wifi wifi;

static GPS gps;
static Accellerometer accel;
static Compass compass;
static Gesture gesture;
static Haptic haptic;
static Audio audio;
static LED led;
static USBFlashDrive flash;
static BLE ble;

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/gpio.h"

// fcohen: These are here because I don't understand C++ classes, static variables, and how to pass callbacks.
// I put the file-related functions into Video.cpp/Video.h and kept the static variables and callbacks
// needed by MjpegClass here. Anyone want to fix this?

#include <Arduino_GFX_Library.h>

static MjpegClass mjpeg;
uint8_t *mjpeg_buf;
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame
boolean firsttime = true;
File vidfile;
int ShowCount = 0;

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
static Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 2 /* rotation */, false /* IPS */);

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  //Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long start = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  video.addTotal_show_video( millis() - start );
  return 1;
}

#include "USB.h"
#include "USBMSC.h"

USBMSC msc;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  //Serial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);   
  return SD.writeRAW( (uint8_t*) buffer, lba) ? bufsize : -1 ;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  //Serial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);    
  return SD.readRAW( (uint8_t*) buffer, lba) ? bufsize : -1 ;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  //Serial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

void testDevices()
{
    if ( storage.testNandStorage() )
  {
    Serial.println( F( " 01 NAND test Success " ) );
  }
  else
  {
    Serial.println( F( " 01 NAND test Failed " ) );    
  }  

  if ( gps.test() )
  {
    Serial.println( F( " 02 GPS test Success " ) );
  }
  else
  {
    Serial.println( F( " 02 GPS test Failed " ) );    
  }

  if ( accel.test() )
  {
    Serial.println( F( " 03 Accelerometer test Success " ) );
  }
  else
  {
    Serial.println( F( " 03 Accelerometer test Failed " ) );    
  }

  if ( compass.test() )
  {
    Serial.println( F( " 04 Compass test Success " ) );
  }
  else
  {
    Serial.println( F( " 04 Compass test Failed " ) );    
  }

/*
  if ( gesture.test() )
  {
    Serial.println( F( " 05 Gesture test Success " ) );
  }
  else
  {
    Serial.println( F( " 05 Gesture test Failed " ) );    
  }
*/

  if ( haptic.test() )
  {
    Serial.println( F( " 06 Haptic test Success " ) );
  }
  else
  {
    Serial.println( F( " 06 Haptic test Failed " ) );    
  }

  if ( audio.test() )
  {
    Serial.println( F( " 07 Audio test Success " ) );
  }
  else
  {
    Serial.println( F( " 07 Audio test Failed " ) );    
  }
}

void startMSC()
{
  Serial.print( F( "card size = " ) );
  Serial.print( SD.cardSize() );
  Serial.print( F( ", numSectors = " ) );
  Serial.print( SD.numSectors() );
  Serial.print( F( ", bytes per sector = " ) );
  Serial.print( SD.cardSize() / SD.numSectors() );
  Serial.print( F( ", total bytes = " ) );
  Serial.print( SD.totalBytes() );
  Serial.print( F( ", usedBytes = " ) );
  Serial.print( SD.usedBytes() );

  Serial.print( F( ", SD Card Type: " ));
  if(SD.cardType() == CARD_MMC){
      Serial.println( F( "MMC" ) );
  } else if(SD.cardType() == CARD_SD){
      Serial.println( F( "SDSC" ) );
  } else if(SD.cardType() == CARD_SDHC){
      Serial.println( F( "SDHC" ) );
  } else if(SD.cardType() == CARD_NONE){
      Serial.println( F( "No SD card attached" ) );
  }else {
      Serial.println( F( "UNKNOWN" ) );
  }
  
  msc.vendorID( "REF32" );
  msc.productID( "USB_MSC" );
  msc.productRevision( "1.0" );
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  msc.begin(SD.numSectors(), SD.cardSize() / SD.numSectors() );

  USB.begin();
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

void setup(){
  Serial.begin(115200);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.setDebugOutput(true);
  Serial.println( F( " " ) );  
  Serial.println( F( " " ) );  
  Serial.println( F( "Reflections Calliope Opens Eyes" ) );

  SPI.begin( SPI_SCK, SPI_MISO, SPI_MOSI );

  Wire.begin( I2CSDA, I2CSCL );

  video.begin();   // Sets pins gets ready to run videos

  gfx->begin();
  gfx->fillScreen( YELLOW );
  gfx->invertDisplay(true);

  storage.begin(); // NAND storage

  //startMSC();     // Calliope mounts as a flash drive, showing NAND contents over USB on your computer
  
  utils.begin();
  shows.begin();
  wifi.begin();
  gps.begin();
  accel.begin();
  compass.begin();
  //gesture.begin();
  haptic.begin();
  audio.begin();
  led.begin();
  flash.begin();

  testDevices();

  ble.begin();
  
  video.setReadyForNextMedia( false );
  video.clearNeedsPlay();

  mjpeg_buf = (uint8_t *) malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println( F("mjpeg_buf malloc failed") );
    while(1);
  }
  
  gfx->fillScreen( BLUE );
  gfx->invertDisplay(true);

  if ( false )
  {
    // Remove all files
    String rootpath = F( "/" );
    File root = SD.open( F( "/" ) );
    storage.rm( root, rootpath );
  
    storage.listDir( SD, "/", 100, true );
    storage.replicateServerFiles();
  }
}


long mylastTime = millis();

void loop(){

  if ( (millis() - mylastTime) > 2000) {
    mylastTime = millis();
    String myh = compass.updateHeading();
    Serial.println( myh );
    ble.setHeading( myh );
  }

  ble.loop();

  // Cooperative multi-tasking functions

  // ArduinoOTA.handle(); 
}

void extraloop(){
  if ( video.getReadyForNextMedia() )
  {  
    if ( shows.findNext() )
    {
      String nv = shows.getNextVideo();
      Serial.print( F("shows.getNextVideo(): " ) );
      Serial.println( nv );

      vidfile = SD.open( shows.getNextVideo(), FILE_READ );
      if ( ! vidfile )
      {
        Serial.print( F( "Could not open: " ) );
        Serial.println( shows.getNextVideo() );
        return;
      }
      
      video.setMjpegFile( vidfile );
      video.setReadyForNextMedia( false );
      video.setNeedsSetup( true );
      video.clearNeedsPlay();
  
      Serial.print( F( "Show " ) );
      Serial.println ( ShowCount++ );
    }
  }

  if ( video.needsSetup() )
  {
    video.clearNeedsSetup();

    File myFile = video.getMjpegFile();
    if ( ! myFile )
    {
      Serial.print( F( "needsSetup myFile did not open" ) );
      while(1);
    }
    else
    {
      Serial.print( F( "myFile = " ) );
      Serial.println( myFile.path() );
    }

    if ( mjpeg.setup(
       &myFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
       0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */, firsttime ) )
    {
      Serial.println( F( "Mjpeg set-up" ) );
      firsttime = false;
    }
    else
    {
      Serial.println( F( "Could not set-up mjpeg") );
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
    smartdelay(100);
  }

  /*
  video.loop();
  storage.loop();
  shows.loop();
  audio.loop();
  utils.loop();
  wifi.loop();
  gps.loop();
  accel.loop();
  compass.loop();
  //gesture.loop();
  haptic.loop();
  led.loop();
  flash.loop();
  */
}
