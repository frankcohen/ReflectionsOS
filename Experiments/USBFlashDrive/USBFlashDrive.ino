/*
Reflections Hoober board as a USB Flash memory stick with contents stored on NAND using ESP32 SD library

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.

This software component works with the Hoober board, the second revision to the main board.
The main board is an ESP32-based platform with OLED display, audio player, flash memory,
GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Arduino IDE settings:
Adafruit Feather ESP32-S3 No PSRAM
USB mode: USB-OTG
USB CDC On Boot: Enabled
USB Firmware MSC On Boot: Disabled
USB DFU On Boot: Disabled

For this to work with Espressif's ESP32 code you need to change line 694
esp32/hardware/eps32/2.0.3/libraries/SD/src/sd_diskio.cpp:

return ff_sd_read(pdrv, buffer, sector, 1) == ESP_OK;

Change '1' to '8'

and line 699 change '1 to '8'

return ff_sd_write(pdrv, buffer, sector, 8) == ESP_OK;

I opened issue: https://github.com/espressif/arduino-esp32/issues/7106

ESP32's SD source is at:
https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/src/SD.cpp

*/

#include "SD.h"
#include "SPI.h"
#include "USB.h"
#include "USBMSC.h"

USBMSC msc;

#define NAND_SPI_MOSI      35
#define NAND_SPI_MISO      37
#define NAND_SPI_SCK       36
#define NAND_SPI_CS        15
#define Display_SPI_CS     12

bool sd_changed = false;
bool sd_inited = false;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  Serial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);   
  return SD.writeRAW( (uint8_t*) buffer, lba) ? bufsize : -1 ;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  Serial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);    
  return SD.readRAW( (uint8_t*) buffer, lba) ? bufsize : -1 ;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  Serial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.println("");
  Serial.println("MSC research for Hoober");

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode( NAND_SPI_CS, OUTPUT );
  digitalWrite( NAND_SPI_CS, LOW);

  SPI.begin( NAND_SPI_SCK, NAND_SPI_MISO, NAND_SPI_MOSI );

  /*
  static SPIClass* spi = NULL;
  spi = new SPIClass(FSPI);
  spi->begin(NAND_SPI_SCK, NAND_SPI_MISO, NAND_SPI_MOSI, NAND_SPI_CS);
  */
                                      
  if ( !SD.begin( NAND_SPI_CS ) )
  //if ( !SD.begin( NAND_SPI_CS, *spi, 20000000 ) )
  {
    Serial.println(F("Storage initialization failed"));
    Serial.println("Stopped");
    while(1);
  }
  else 
  {
    Serial.println(F("Storage initialization success"));
  }

  Serial.print( "card size = " );
  Serial.print( SD.cardSize() );
  Serial.print( ", numSectors = " );
  Serial.print( SD.numSectors() );
  Serial.print( ", bytes per sector = " );
  Serial.print( SD.cardSize() / SD.numSectors() );
  Serial.print( ", total bytes = " );
  Serial.print( SD.totalBytes() );
  Serial.print( ", usedBytes = " );
  Serial.print( SD.usedBytes() );

  Serial.print(", SD Card Type: ");
  if(SD.cardType() == CARD_MMC){
      Serial.println("MMC");
  } else if(SD.cardType() == CARD_SD){
      Serial.println("SDSC");
  } else if(SD.cardType() == CARD_SDHC){
      Serial.println("SDHC");
  } else if(SD.cardType() == CARD_NONE){
      Serial.println("No SD card attached");
  }else {
      Serial.println("UNKNOWN");
  }

  msc.vendorID("REF32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);
  msc.begin(SD.numSectors(), SD.cardSize() / SD.numSectors() );

  USB.begin();

  sd_changed = true; // to print contents initially
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void loop() {
  if ( sd_changed )
  {
    Serial.println("SD contents:");
    listDir(SD, "/", 0);
    listDir(SD, "/3a0b4955fb50992baca383a837849987", 0);
    
    Serial.println();
    sd_changed = false;
  }

  delay(1000); // refresh every 1 second
}
