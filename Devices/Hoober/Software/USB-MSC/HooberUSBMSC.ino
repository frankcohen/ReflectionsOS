/*
Reflections USB MSC for Hoober research

This sketch turns the Reflections main board into a USB PCMCIA flash mass storage device. 
Connect the board to the USB port of your Mac or other laptop and the NAND SD card
contents appear as a disk drive. This is something Limor Fried (@LadyAda) and Phillip Torrone (@pt)
first demonstrated in 2019 on https://www.youtube.com/watch?v=0bWba0PU4-g
Unfortunately, Adafruit_TinyUSB does not compile under the ESP32-S3.
This sketch makes their idea work on an ESP32-S3-Mini-1 with a NAND storage device
and USB port connected directly to the ESP32. NAND is a surface mount version of an 
SD mass storage card. NAND gives gigabytes of storage. ESP32 uses the SPI bus to
communicate with the NAND. This sketch uses the TinyUSB library support included in 
the ESP32 libraries. Reflections software and hardware is distributed under a GPL v3 Open Source license.
See the Reflections Wiring Guide for details at:
https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Hoober/Software/USB-MSC/USB-MSC-Wiring-Guide.jpg

When uploading this in Arduino IDE these need selection in the Tools drop-down menu:
USB Mode: USB-OTG
USB CDC On Boot: Enabled
USB Firmware MSC On Boot: Disabled
USB DFU On Boot: Disabled

Depends on:
 * SdFat https://github.com/adafruit/SdFat
 * ESP32 TinyUSB implementation (USBMSC, USB) part of the Arduino IDE hardware library

Built from examples:
https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/master/examples/MassStorage/msc_external_flash_sdcard/msc_external_flash_sdcard.ino

About Reflections:
What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is one of the software components. It runs on an ESP32-based platform with 
TFT OLED display, audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

*/

#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

#include "USBMSC.h"

// Hoober MOSI is 11, Feather 35, feather2 12
#define SPI_MOSI      11
// Hoober MISO 13, Feather 37, feather2 11
#define SPI_MISO      13
// Hoober SCK 12, Feather 36, feather2 10
#define SPI_SCK       12
// Hoober CS 16, Feather 10, feather2 13
#define SD_CS         16

static const uint32_t DISK_SECTOR_COUNT = 240 * 1000; // 8KB is the smallest size that windows allow to mount
static const uint16_t DISK_SECTOR_SIZE = 512;    // Should be 512
static const uint16_t DISC_SECTORS_PER_TABLE = 1; //each table sector can fit 170KB (340 sectors)

SdFat sd;

USBMSC usb_msc;   // USB Mass Storage object

// Set to true when PC write to flash
bool sd_changed = false;
bool sd_inited = false;

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("Reflections USB MSC for Hoober research");
  //Serial.print("Waited ");
  //Serial.println( millis() - time );
  
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, LOW);

  // A workaround to a problem found in the Hoober board
  pinMode(42, OUTPUT);
  digitalWrite(42, LOW);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);  
  
  init_sdcard();
  sd_inited = true;
    
  Serial.println("USB MSC ready");
  delay(1000);
}


bool init_sdcard(void)
{
  Serial.print("Init NAND SD storage ... ");

  if ( !sd.begin( SD_CS ) )
  {
    Serial.print("failed ");
    sd.errorPrint();
    Serial.println("");
    return false;
  }
  Serial.println("success");

  sd_changed = true; // to print contents initially

  // Calculate free space (volume free clusters * blocks per clusters / 2)
  long lFreeKB = sd.vol()->freeClusterCount();
  lFreeKB *= sd.vol()->blocksPerCluster()/2;
  Serial.print("Free space: ");
  Serial.print( lFreeKB / 1024 );
  Serial.println(" MB");

  Serial.print("DISK_SECTOR_COUNT: ");
  Serial.println( DISK_SECTOR_COUNT );
  Serial.print( "DISK_SECTOR_SIZE: " );
  Serial.println( DISK_SECTOR_SIZE );

  usb_msc.vendorID("Reflect");//max 8 chars
  usb_msc.productID("MSC");//max 16 chars
  usb_msc.productRevision("1.1");//max 4 chars
  usb_msc.onStartStop(onStartStop);
  usb_msc.onRead(onRead);
  usb_msc.onWrite(onWrite);
  usb_msc.mediaPresent(true);
  usb_msc.begin(DISK_SECTOR_COUNT, DISK_SECTOR_SIZE);
  
  return true;
}

void print_rootdir(File* rdir)
{
  File file;

  // Open next file in root.
  // Warning, openNext starts at the current directory position
  // so a rewind of the directory may be required.
  while ( file.openNext(rdir, O_RDONLY) )
  {
    file.printFileSize(&Serial);
    Serial.write(' ');
    file.printName(&Serial);
    if ( file.isDir() )
    {
      // Indicate a directory.
      Serial.write('/');
    }
    Serial.println();
    file.close();
  }
}

void loop()
{
  if ( sd_changed )
  {
    File root;
    root = sd.open("/");

    Serial.println("SD contents:");
    print_rootdir(&root);
    Serial.println();

    root.close();

    sd_changed = false;
  }

  delay(1000); // refresh every 1 second
}


//--------------------------------------------------------------------+
// SD Card callbacks
//--------------------------------------------------------------------+

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  Serial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  return sd.card()->readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  digitalWrite(LED_BUILTIN, HIGH);

  return sd.card()->writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void sdcard_flush_cb (void)
{
  sd.card()->syncBlocks();

  // clear file system's cache to force refresh
  sd.cacheClear();

  sd_changed = true;

  digitalWrite(LED_BUILTIN, LOW);
}
