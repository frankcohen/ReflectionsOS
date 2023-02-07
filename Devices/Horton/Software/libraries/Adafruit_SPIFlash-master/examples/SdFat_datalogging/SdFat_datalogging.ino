// Adafruit SPI Flash FatFs Simple Datalogging Example
// Author: Tony DiCola
//
// This is a simple dataloging example using the SPI Flash
// FatFs library.  The example will open a file on the SPI
// flash and append new lines of data every minute. Note that
// you MUST have a flash chip that's formatted with a flash
// filesystem before running.  See the fatfs_format example
// to perform this formatting.
//
// Usage:
// - Modify the pins and type of fatfs object in the config
//   section below if necessary (usually not necessary).
// - Upload this sketch to your M0 express board.
// - Open the serial monitor at 115200 baud.  You should see the
//   example print a message every minute when it writes a new
//   value to the data logging file.
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>

#if defined(ARDUINO_ARCH_ESP32)
  // ESP32 use same flash device that store code.
  // Therefore there is no need to specify the SPI and SS
  Adafruit_FlashTransport_ESP32 flashTransport;

#elif defined(ARDUINO_ARCH_RP2040)
  // RP2040 use same flash device that store code.
  // Therefore there is no need to specify the SPI and SS
  // Use default (no-args) constructor to be compatible with CircuitPython partition scheme
  Adafruit_FlashTransport_RP2040 flashTransport;

  // For generic usage: Adafruit_FlashTransport_RP2040(start_address, size)
  // If start_address and size are both 0, value that match filesystem setting in
  // 'Tools->Flash Size' menu selection will be used

#else
  // On-board external flash (QSPI or SPI) macros should already
  // defined in your board variant if supported
  // - EXTERNAL_FLASH_USE_QSPI
  // - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
  #if defined(EXTERNAL_FLASH_USE_QSPI)
    Adafruit_FlashTransport_QSPI flashTransport;

  #elif defined(EXTERNAL_FLASH_USE_SPI)
    Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

  #else
    #error No QSPI/SPI flash are defined on your board variant.h !
  #endif
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

// Configuration for the datalogging file:
#define FILE_NAME      "data.csv"


void setup() {
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("Adafruit SPI Flash FatFs Simple Datalogging Example");

  // Initialize flash library and check its chip ID.
  if (!flash.begin()) {
    Serial.println("Error, failed to initialize flash chip!");
    while(1) delay(1);
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);

  // First call begin to mount the filesystem.  Check that it returns true
  // to make sure the filesystem was mounted.
  if (!fatfs.begin(&flash)) {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    Serial.println("Was the flash chip formatted with the fatfs_format example?");
    while(1) delay(1);
  }
  Serial.println("Mounted filesystem!");

  Serial.println("Logging data every 60 seconds...");
}

void loop() {
  // Open the datalogging file for writing.  The FILE_WRITE mode will open
  // the file for appending, i.e. it will add new data to the end of the file.
  File dataFile = fatfs.open(FILE_NAME, FILE_WRITE);
  // Check that the file opened successfully and write a line to it.
  if (dataFile) {
    // Take a new data reading from a sensor, etc.  For this example just
    // make up a random number.
    int reading = random(0,100);
    // Write a line to the file.  You can use all the same print functions
    // as if you're writing to the serial monitor.  For example to write
    // two CSV (commas separated) values:
    dataFile.print("Sensor #1");
    dataFile.print(",");
    dataFile.print(reading, DEC);
    dataFile.println();
    // Finally close the file when done writing.  This is smart to do to make
    // sure all the data is written to the file.
    dataFile.close();
    Serial.println("Wrote new measurement to data file!");
  }
  else {
    Serial.println("Failed to open data file for writing!");
  }

  Serial.println("Trying again in 60 seconds...");

  // Wait 60 seconds.
  delay(60000L);
}
