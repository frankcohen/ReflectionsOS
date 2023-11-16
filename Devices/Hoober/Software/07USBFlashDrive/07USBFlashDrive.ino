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

// Display
#define Display_SPI_DC    GPIO_NUM_5
#define Display_SPI_CS    GPIO_NUM_12
#define Display_SPI_RST   GPIO_NUM_0
#define Display_SPI_BK    GPIO_NUM_6

static const uint32_t DISK_SECTOR_COUNT = 240 * 1000; // 8KB is the smallest size that windows allow to mount
static const uint16_t DISK_SECTOR_SIZE = 512;    // Should be 512
static const uint16_t DISC_SECTORS_PER_TABLE = 1; //each table sector can fit 170KB (340 sectors)

bool sd_changed = false;
bool sd_inited = false;

int crlf = 0;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  if ( crlf++ > 30 )
  {
    Serial.println( "w" );
    crlf = 0;   
  }
  else
  {
    Serial.print( "w" );    
  }
  SD.writeRAW((uint8_t*)buffer, lba);
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  if ( crlf++ > 30 )
  {
    Serial.println("r" );
    crlf = 0;   
  }
  else
  {
    Serial.print( "r" );    
  }
  SD.readRAW((uint8_t*)buffer, lba);
  return bufsize;    
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  Serial.println("");
  Serial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

static void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
  if(event_base == ARDUINO_USB_EVENTS){
    arduino_usb_event_data_t * data = (arduino_usb_event_data_t*)event_data;
    switch (event_id){
      case ARDUINO_USB_STARTED_EVENT:
        Serial.println("");
        Serial.println("USB PLUGGED");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        Serial.println("");
        Serial.println("USB UNPLUGGED");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        Serial.println("");
        Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
        break;
      case ARDUINO_USB_RESUME_EVENT:
        Serial.println("");
        Serial.println("USB RESUMED");
        break;
      
      default:
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.println("");
  Serial.println("MSC research for Hoober");

  pinMode( NAND_SPI_CS, OUTPUT );
  digitalWrite(NAND_SPI_CS, LOW);

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, LOW);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);

  SPI.begin(NAND_SPI_SCK, NAND_SPI_MISO, NAND_SPI_MOSI);  
  if ( !SD.begin( NAND_SPI_CS ) )
  {
    Serial.println(F("Storage initialization failed"));
    Serial.println("Stopped");
    while(1);
  }
  else 
  {
    Serial.println(F("Storage initialization success"));
  }

  msc.vendorID("REF32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);

  Serial.print("Disk sector count: ");
  Serial.print( DISK_SECTOR_COUNT );
  Serial.print( ", disk sector size: " );
  Serial.println( DISK_SECTOR_SIZE );
  msc.begin(DISK_SECTOR_COUNT, DISK_SECTOR_SIZE);

  USB.onEvent(usbEventCallback);
  USB.begin();

  if(SD.cardType() == CARD_NONE)
  {
    Serial.println(", No SD card attached");
    return;
  }

  sd_changed = true; // to print contents initially  

  Serial.print( "total bytes = " );
  Serial.print( SD.totalBytes() );
  Serial.print( ", card size = " );
  Serial.print( SD.cardSize() );
  Serial.print( ", numSectors = " );
  Serial.print( SD.numSectors() );
  Serial.print( ", usedBytes = " );
  Serial.print( SD.usedBytes() );

  Serial.print(", SD Card Type: ");
  if(SD.cardType() == CARD_MMC){
      Serial.println("MMC");
  } else if(SD.cardType() == CARD_SD){
      Serial.println("SDSC");
  } else if(SD.cardType() == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }
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
    Serial.println();
    sd_changed = false;
  }

  delay(1000); // refresh every 1 second
}
