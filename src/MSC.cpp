/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "MSC.h"

MSC::MSC(){}

void MSC::begin()
{ 
}

void MSC::loop()
{
}

/* 

USB MSC disabled, I didn't figure out how to get it to work correctly:
https://github.com/espressif/arduino-esp32/issues/7106

I may rewrite this around:
https://github.com/espressif/esp-idf/blob/4e04c00d1ca6d47c2914c7cd2e1fe66d46ce1d3b/examples/peripherals/usb/host/msc/main/msc_example_main.c

I am concerned about this solution. It requires the RTOS multitasking kernel.
I prefer cooperative multi-tasking to make sure the video/audio of the 
entertaining experience runs without pre-emptive multitasking's possible
drop-offs. 

For now I am disabling MSC. The Storage class implements a file replication 
mechanism to get files onto the SD/NAND device.

// USB Mass Storage, makes SD/NAND appear as a drive on desktop/laptop computers

// #include "USB.h"
// #include "USBMSC.h"

// USBMSC msc;

/*
static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  //Serial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);   

  String msg = "MSC WRITE: lba: ";
  msg += lba;
  msg += ", offset: ";
  msg += offset;
  msg += ", bufsize: ";
  msg += bufsize;
  msg += "\0";
  logger.info( msg );

  return SD.writeRAW( (uint8_t*) buffer, lba) ? bufsize : -1 ;
}
*/

/*
static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  //Serial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);    

  String msg = "MSC READ: lba: ";
  msg += lba;
  msg += ", offset: ";
  msg += offset;
  msg += ", bufsize: ";
  msg += "\0";
  logger.info( msg );
  
  return SD.readRAW( (uint8_t*) buffer, lba) ? bufsize : -1 ;
}
*/

/*
static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  //Serial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  
  String msg = "MSC START/STOP: power: ";
  msg += power_condition;
  msg += ", start: ";
  msg += start;
  msg += ", ejection: ";
  msg += load_eject;
  msg += "\0";
  logger.info( msg );

  return true;
}
*/

/* MSC mounts the SD/NAND to a desktop computer as a hard drive volume */

/*

void startMSC()
{
  logger.info( F( "Starting USB MSC service" ) );

  String msg = F( "card size = " );
  msg += SD.cardSize();
  msg += F( ", numSectors = " );
  msg += SD.numSectors();
  msg += F( ", bytes per sector = " );
  msg += SD.cardSize() / SD.numSectors();
  msg += F( ", total bytes = " );
  msg += SD.totalBytes();
  msg += F( ", usedBytes = " );
  msg += SD.usedBytes();

  msg += F( ", SD Card Type: " );
  if( SD.cardType() == CARD_MMC){
      msg += F( "MMC" );
  } else if(SD.cardType() == CARD_SD){
      msg += F( "SDSC" );
  } else if(SD.cardType() == CARD_SDHC){
      msg += F( "SDHC" );
  } else if(SD.cardType() == CARD_NONE){
      msg += F( "No SD card attached" );
  }else {
      msg += F( "UNKNOWN" );
  }
  
  logger.info( msg );
  
  msc.vendorID( "ROF32" );
  msc.productID( "USB_MSC" );
  msc.productRevision( "1.0" );
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  msc.begin(SD.numSectors(), SD.cardSize() / SD.numSectors() );

  USB.begin();
}
*/
