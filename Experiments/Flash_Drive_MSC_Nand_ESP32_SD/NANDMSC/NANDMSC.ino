/*
Operates the Reflections board NAND/SD as a USB flash drive

Documentation, schematic, and source code at [Experiments/Flash_Drive_MSC_Nand_ESP32_SD/NANDMSC](https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Flash_Drive_MSC_Nand_ESP32_SD/NANDMSC)

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with display,
audio player, flash memory, GPS, gesture sensor, accelerometer/compass, and more.

Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS)
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Based on code from
https://www.youtube.com/@atomic14 
and
https://github.com/atomic14/esp32-sdcard-msc/tree/main
SD MSC utility for ESP32 processors

Arduino IDE settings:
Adafruit Feather ESP32-S3 No PSRAM
USB mode: USB-OTG
USB CDC On Boot: Enabled
USB Firmware MSC On Boot: Disabled
USB DFU On Boot: Disabled
JTAG Adapter: Integrated USB JTAG

*/

#include <Arduino.h>
#include "SD.h"
#include "SPI.h"
#include "USB.h"
#include "USBMSC.h"

#include "SDCardArduino.h"
#include "SDCardLazyWrite.h"

#define SD_CS_PIN GPIO_NUM_15

#define SPI_MOSI      GPIO_NUM_35
#define SPI_MISO      GPIO_NUM_37
#define SPI_SCK       GPIO_NUM_36

// Display
#define Display_SPI_DC    GPIO_NUM_5
#define Display_SPI_CS    GPIO_NUM_12
#define Display_SPI_RST   GPIO_NUM_0
#define Display_SPI_BK    GPIO_NUM_6

USBMSC msc;
USBCDC SerialCDC;
SDCard *card;

void log(const char *str)
{
  SerialCDC.println(str);
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
  Serial.printf("Writing %d bytes to %d at offset\n", bufsize, lba, offset);
  // this writes a complete sector so we should return sector size on success
  if (card->writeSectors(buffer, lba, bufsize / card->getSectorSize()))
  {
    return bufsize;
  }
  return bufsize;
  // return -1;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  SerialCDC.printf("Reading %d bytes from %d at offset %d\n", bufsize, lba, offset);
  // this reads a complete sector so we should return sector size on success
  if (card->readSectors((uint8_t *)buffer, lba, bufsize / card->getSectorSize()))
  {
    return bufsize;
  }
  return -1;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject)
{
  SerialCDC.printf("StartStop: %d %d %d\n", power_condition, start, load_eject);
  if (load_eject)
  {
    SerialCDC.printf( "load_eject" );
  }
  return true;
}

void setup() {

  Serial.begin(115200);
  delay(2000);
  Serial.println("beginning");
  delay(200);
  Serial.println("beginning");
  delay(200);
  Serial.println("beginning");

  // Configures Reflections Blue Fish board
  // https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/BlueFish
  
  pinMode(SD_CS_PIN, OUTPUT );
  digitalWrite(SD_CS_PIN, LOW);

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, LOW);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);

  card = new SDCardArduino(Serial, "/sd", SPI_MISO, SPI_MOSI, SPI_SCK, SD_CS_PIN);

  card->printCardInfo();
  Serial.printf("Sector Size=%d\n", card->getSectorSize());

  msc.vendorID("ESP32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);

  msc.begin(card->getSectorCount(), card->getSectorSize());
  USB.begin();

  Serial.println( "NANDMSC started" );
}

int i = 0;

void loop() {
  delay(2000);
  Serial.println( i++ );
}
