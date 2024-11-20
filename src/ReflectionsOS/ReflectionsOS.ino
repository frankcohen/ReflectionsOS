/*
Reflections is a hardware and software platform for building entertaining mobile experiences.

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with display,
audio player, flash memory, GPS, gesture sensor, accelerometer/compass, and more.

This code is the operating system and an example wrist watch experience. Use and
ignore whatever elements of the example may be used for your experience.

Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS/)
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Depends on these libraries:

For convenience, these libraries are in the [https://github.com/frankcohen/ReflectionsOS/libraries](https://github.com/frankcohen/ReflectionsOS)
directory. Copy the contents to your Arduino IDE installation under documents/libraries.

ESP32 board, https://github.com/espressif/arduino-esp32
esp32FOTA, OTA updates, https://github.com/chrisjoyce911/esp32FOTA
Adafruit DRV2605 Library, haptic controller, https://github.com/adafruit/Adafruit_DRV2605_Library
Sparkfun LIS3DH, https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library

Adafruit MMC56x3, compass, magnetometer, https://github.com/adafruit/Adafruit_MMC56x3
The Adafruit I2C libraries, like MMC56x3, depend on these
Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
Adafruit SSD1306, compass support, https://github.com/adafruit/Adafruit_SSD1306

Adafruit seesaw Library, https://github.com/adafruit/Adafruit_Seesaw
ArduinoJson, https://arduinojson.org/
ESP32-targz, https://github.com/tobozo/ESP32-targz/
ESP32_HTTPS_Server, https://github.com/fhessel/esp32_https_server
ESP8266Audio, https://github.com/earlephilhower/ESP8266Audio
FastLED, https://github.com/FastLED/FastLED
GFX Library for Arduino, https://github.com/moononournation/Arduino_GFX
JPEGDEC, https://github.com/bitbank2/JPEGDEC.git
SparkFun_VL53L5CX_Arduino_Library, https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
Time, https://playground.arduino.cc/Code/Time/
TinyGPSPlus-ESP32, https://github.com/Tinyu-Zhao/TinyGPSPlus-ESP32
URLEncode, https://github.com/plageoj/urlencode
AutoConnect by Hieromon, https://github.com/Hieromon/AutoConnect
PNGdec library, https://github.com/bitbank2/PNGdec

Depends on Esspresif ESP32 libraries at
[https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)

Using Arduino IDE 2.x, use Tools menu options:
Board: ESP32S3 Dev Module
CPU Frequency: 240Mhz (Wifi)
USB CDC On Boot: Enabled
Core Debug Level: Error
USB DFU On Boot: Disabled
Erase All Flash Before Sketch Upload: Disabled
Events Run On: Core 1
Flash Mode: QIO 80 Mhz
Flash Size: 8 MB (64MB)
JTAG Adapter: Integrated USB JTAG
Arduino Runs On: Core 1
USB Firmware MSC On Boot: Disabled
Partition Scheme: Reflections App (8MB OTA No SPIFFS)    See below
PSRAM: Disabled
Upload Mode: UART0/Hardware CDC
Upload Speed 921600
USB Mode: Hardware CDC and JTAG
Zigbee Mode: Disabled

Reflections uses a custom partition scheme to maximize available flash storage.
See instructions at: https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/Partition%20tables%20and%20optimizing%20memory%20in%20Arduino%20IDE.md
Arduino IDE 2.x automatically uses partitions.csv in the source code directory

*/

#include "Arduino.h"
//#include "BLE.h"
#include "Utils.h"
#include "config.h"
#include "Storage.h"
#include "Logger.h"
#include "Battery.h"
#include "Video.h"
#include "MjpegClass.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include "secrets.h"
#include "Accelerometer.h"
#include "Audio.h"
#include "Compass.h"
#include "GPS.h"
#include "TOF.h"
#include "Haptic.h"
#include "LED.h"
#include "USBFlashDrive.h"
// #include "OTA.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/gpio.h"
#include "Hardware.h"
#include "Wire.h"
#include "Parallax.h"
#include "TextMessageService.h"
#include "ExperienceService.h"
#include <Arduino_GFX_Library.h>
#include "WatchFaceExperiences.h"
#include "nvs_flash.h"
#include "RealTimeClock.h"

Video video;
Utils utils;
Storage storage;
Haptic haptic;
Audio audio;
TOF tof;
//Parallax parallax;
TextMessageService textmessageservice;
ExperienceService experienceservice;
GPS gps;
Wifi wifi;
Compass compass;
//LED led;
//USBFlashDrive flash;
//OTA ota;
LOGGER logger;
Battery battery;
Hardware hardware;
Accelerometer accel;
//BLEServerClass bleServer;
//BLEClientClass bleClient;
WatchFaceExperiences watchFaceExperiences;
RealTimeClock realtimeclock;

const char *root_ca = ssl_cert;  // Shared instance of the server side SSL certificate, found in secrets.h

// Host name
std::string devname;
String devicename;

// Timers

unsigned long startvidtime;
bool startvidflag;
int startcnt;

bool tofstarted;

/* Test for a device number on the I2C bus, display error when not found */

void assertI2Cdevice(byte deviceNum, String devName) {

  for ( int i = 0; i < 10; i++ )
  {
    Wire.beginTransmission(deviceNum);
    if (Wire.endTransmission() == 0)
    {
      return;
    }
    delay( 2000 );
  }

  Serial.print( devName );
  Serial.println( F( " not found." ) );

  video.stopOnError(devName, "not found", "", "", "");
}

/*
  Cooperative multi-tasking functions
*/

static void smartdelay( unsigned long ms )
{
  unsigned long start = millis();

  do 
  {
    // Device operations
    
    video.loop();
    battery.loop();
    //accel.loop();
    storage.loop();
    wifi.loop();  
    utils.loop();
    compass.loop();
    haptic.loop();
    //led.loop();
    //bleClient.loop();
    realtimeclock.loop();

    // Watch experience operations

    watchFaceExperiences.loop();
    
    //experienceservice.loop();
    //TextMessageService.loop();
    
    /*
    logger.loop();
    parallax.loop();
    audio.loop();
    */

  } while (millis() - start < ms);
}

void setup() {
  Serial.begin(115200);
  long time = millis();
  while (!Serial && (millis() < time + 2000)) ;  // wait up to 2 seconds for Arduino Serial Monitor
  Serial.setDebugOutput(true);

  Serial.println("Starting");

  //wifi.reset();  // Optionally reset any previous connection settings
  //wifi.begin();  // Non-blocking, until guest uses it to connect

  realtimeclock.begin();
  
  hardware.begin();   // Sets all the hardware pins

  storage.begin();
  storage.setMounted( hardware.getMounted() );

  //storage.replicateServerFiles();
  
  //Serial.println( "Files on board:" );
  //storage.listDir(SD, "/", 100, true);

  Serial.println("- - -");
  Serial.println(F("ReflectionsOS"));
  Serial.println(" ");

  video.begin();

  //utils.WireScan();
/*
  Serial.println( "nvs_flash_init()" );
  nvs_flash_erase();
  esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        Serial.println( "nvs_flash_erase()" );
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    Serial.println( "nvs done" );
*/

  logger.begin();
  logger.setEchoToSerial( true );
  logger.setEchoToServer( false );

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append(mac.substr(15, 2));
  devicename = devname.c_str();
  String hostinfo = "Host: ";
  hostinfo += devicename;
  logger.info(hostinfo);  

  // Self-test: NAND, I2C, SPI

  assertI2Cdevice(24, "Acclerometer");
  assertI2Cdevice(48, "Compass");
  assertI2Cdevice(90, "Haptic");
  assertI2Cdevice(41, "TOF Accel");

  // Device initialization

  haptic.begin();
  battery.begin();
  audio.begin();
  gps.begin();
  accel.begin();
  compass.begin();
  //parallax.begin();
  textmessageservice.begin();
  experienceservice.begin();
  utils.begin();
  //led.begin();

  // Watch experience initialization

  watchFaceExperiences.begin();

  //bleServer.begin();  // Initializes the BLE server
  //bleClient.begin();  // Initializes the BLE client

  //accel.setTraining( true );    // Put accelermoeter into training mode
  //accel.loadGestures();         // Load the prerecorded accelermeter gestures

  //ota.begin();
  //ota.update();     // Just in case previous use of the host replicated an OTA update file

  //startMSC();     // Calliope mounts as a flash drive, showing NAND contents over USB on your computer

/*
  tofstarted = true;

  // Create a new task for TOF processing, pin it to core 0
  xTaskCreatePinnedToCore(
    TOFTask,   // Task function
    "TOFTask", // Name of the task
    10000,     // Stack size (in words, not bytes)
    NULL,      // Task input parameter
    1,         // Priority of the task
    NULL,      // Task handle
    0          // Core where the task should run (core 0)
  );
*/

  haptic.playEffect(14);  // 14 Strong Buzz

  startvidtime = millis();
  startvidflag = true;
  
  logger.info(F("Setup complete"));
}

/* Runs TOF gesture sensor in core 0 */

void TOFTask(void *pvParameters) 
{
  while (true) 
  {
    if ( tofstarted ) 
    {
      tof.begin();
      tofstarted = false;
    }

    tof.loop(); // Process and update gesture data
    // Delay to prevent task from monopolizing the CPU
    vTaskDelay( pdMS_TO_TICKS(10) ); // Delay for 10 milliseconds
  }
}

void loop() {
  smartdelay(5000);
  const char* fmtMemCk = "Free: %d\tMaxAlloc: %d\t PSFree: %d\n";
  #define MEMCK Serial.printf(fmtMemCk,ESP.getFreeHeap(),ESP.getMaxAllocHeap(),ESP.getFreePsram())
}
