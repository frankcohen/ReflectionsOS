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
Adafruit LIS3DH library, https://github.com/adafruit/Adafruit_LIS3DH
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
Partition Scheme: Custom, Reflections App (8MB OTA No SPIFFS)    See below
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
#include "AccelSensor.h"
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
#include "WatchFaceMain.h"
#include "nvs_flash.h"
#include "RealTimeClock.h"
#include "Steps.h"
#include "TimerService.h"
#include "SystemLoad.h"

Video video;
Utils utils;
Storage storage;
Haptic haptic;
Audio audio;
TOF tof;
TextMessageService textmessageservice;
ExperienceService experienceservice;
GPS gps;
Wifi wifi;
Compass compass;
LOGGER logger;
Battery battery;
Hardware hardware;
AccelSensor accel;
WatchFaceExperiences watchfaceexperiences;
WatchFaceMain watchfacemain;
RealTimeClock realtimeclock;
Steps steps;
TimerService timerservice;
SystemLoad systemload;

//Parallax parallax;
//LED led;
//USBFlashDrive flash;
//OTA ota;
//BLEServerClass bleServer;
//BLEClientClass bleClient;

const char *root_ca = ssl_cert;  // Shared instance of the server side SSL certificate, found in secrets.h

// Host name
std::string devname;
String devicename;

bool tofstarted = false;
bool accelstarted = false;

/* Test for a device number on the I2C bus, display error when not found */

void assertI2Cdevice(byte deviceNum, String devName) {

  for (int i = 0; i < 10; i++) {
    Wire.beginTransmission(deviceNum);
    if (Wire.endTransmission() == 0) {
      return;
    }
    delay(2000);
  }

  Serial.print(devName);
  Serial.println(F(" not found."));

  video.stopOnError(devName, "not found", "", "", "");
}

/*
  Cooperative multi-tasking functions
*/

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  unsigned long tasktime;

  do {
    tasktime = millis();

    // Device operations

    //video.loop();
    battery.loop();
    storage.loop();
    wifi.loop();
    utils.loop();
    compass.loop();
    haptic.loop();
    //led.loop();
    //bleClient.loop();
    realtimeclock.loop();
    gps.loop();
    steps.loop();
    timerservice.loop();

    // Watch experience operations

/* testing TOF
    unsigned long fellow = millis();
    watchfaceexperiences.loop();
    systemload.logtasktime(millis() - fellow, 1, "we");
    fellow = millis();
    experienceservice.loop();
    systemload.logtasktime(millis() - fellow, 2, "ex");
    fellow = millis();
    textmessageservice.loop();
    systemload.logtasktime(millis() - fellow, 3, "tm");
*/

    //systemload.loop();

    /*
    logger.loop();
    parallax.loop();
    audio.loop();
    */

/*
    if ( watchfaceexperiences.okToSleep() )
    {
      Serial.println( "Light sleep" );

      esp_sleep_enable_timer_wakeup( 100000 * 3 );  // Time in microseconds, 3 = 300 milliseconds ms
      esp_light_sleep_start();                      // Enter light sleep mode
    }
*/

    systemload.logtasktime(millis() - tasktime, 0, "");
  } while (millis() - start < ms);
}

void setup() {
  Serial.begin(115200);
  long time = millis();
  while (!Serial && (millis() < time + 2000))
    ;  // wait up to 2 seconds for Arduino Serial Monitor
  Serial.setDebugOutput(true);

  Serial.println(" ");
  Serial.println("Starting");
  Serial.println(F("ReflectionsOS"));

  // Core 1 services

  systemload.begin();  // System load monitor

  hardware.begin();  // Sets all the hardware pins

  storage.begin();
  storage.setMounted(hardware.getMounted());

  //wifi.reset();  // Optionally reset any previous connection settings
  //wifi.begin();  // Non-blocking, until guest uses it to connect
  //storage.replicateServerFiles();

  //Serial.println( "Files:" );
  //storage.listDir(SD, "/", 100, true);

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
  logger.setEchoToSerial(true);
  logger.setEchoToServer(false);

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
  utils.begin();

  // Support service initialization

  steps.begin();
  timerservice.begin();
  realtimeclock.begin();

  // Core 0 services

  tofstarted = false;
  accelstarted = false;

  // Create a new task for TOF processing, pin it to core 0
  xTaskCreatePinnedToCore(
    Core0Tasks,    // Task function
    "Core0Tasks",  // Name of the task
    10000,         // Stack size (in words, not bytes)
    NULL,          // Task input parameter
    1,             // Priority of the task
    NULL,          // Task handle
    0              // Core where the task should run (core 0)
  );

  // Unused services

  //bleServer.begin();  // Initializes the BLE server
  //bleClient.begin();  // Initializes the BLE client

  //ota.begin();
  //ota.update();     // Just in case previous use of the host replicated an OTA update file

  //startMSC();     // Calliope mounts as a flash drive, showing NAND contents over USB on your computer

  // Experience initialization

  textmessageservice.begin();
  experienceservice.begin();
  watchfaceexperiences.begin();

  // Unused experiences

  //parallax.begin();
  //led.begin();

  //haptic.playEffect(14);  // 14 Strong Buzz

  logger.info(F("Setup complete"));
}

/* Runs TOF gesture sensor in core 0 */

void Core0Tasks(void *pvParameters) {
  while (true) {
    if (!tofstarted) {
      tof.begin();
      tofstarted = true;
    } else {
      tof.loop(); // Process and update gesture data
    }

    if (!accelstarted) {
      accel.begin();
      accelstarted = true;
    } else {
      //accel.loop();
    }

    // Delay to prevent task from monopolizing the CPU
    vTaskDelay(pdMS_TO_TICKS(10));  // Delay for 10 milliseconds
  }
}

unsigned long slowman = millis();
int rowCount = 0;
unsigned long statstime = millis();

void loop() 
{

  if ( millis() - statstime > 1500 )
  {
    statstime = millis();

    //Serial.println( tof.getStats() );
    String mf = tof.getMef();
    String mf2 = tof.getMef2();

    if ( mf != "" ) Serial.println( mf );    
    if ( mf2 != "" ) Serial.println( mf2 );
  }

  // Printing accelerometer statistics here because this code runs in Core 1
  // otherwise the stats compete for the Serial monitor

  if ( millis() - slowman > 500 )
  {
    slowman = millis();

     rowCount++;
    // Every 10 rows, reprint the header
    if (rowCount >= 10) 
    {
      rowCount = 0;  // Reset the counter
//      Serial.println( accel.printHeader() );  // Reprint the header
    }

    //Serial.print( "dtime " );
    //Serial.println( accel.getmostrecentdoubletaptime() );

    //Serial.println( accel.printValues() );
  }

  smartdelay(500);
}
