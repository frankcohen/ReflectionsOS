/*
Reflections, distributed entertainment platform

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with display,
audio player, flash memory, GPS, gesture sensor, accelerometer/compass, and more.

Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS)
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Depends on these libraries:

For convenience, these libraries are in the [https://github.com/frankcohen/ReflectionsOS/libraries](https://github.com/frankcohen/ReflectionsOS)
directory. Copy the contents to your Arduino IDE installation under documents/libraries.

esp32FOTA, OTA updates, https://github.com/chrisjoyce911/esp32FOTA
Adafruit DRV2605 Library, haptic controller, https://github.com/adafruit/Adafruit_DRV2605_Library
Adafruit MMC56x3, compass, magnetometer, https://github.com/adafruit/Adafruit_MMC56x3
Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
Adafruit seesaw Library, https://github.com/adafruit/Adafruit_Seesaw
Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
ArduinoJson, https://arduinojson.org/
ESP32-targz, https://github.com/tobozo/ESP32-targz/
ESP32_HTTPS_Server, https://github.com/fhessel/esp32_https_server
ESP8266Audio, https://github.com/earlephilhower/ESP8266Audio
FastLED, https://github.com/FastLED/FastLED
GFX Library for Arduino, https://github.com/moononournation/Arduino_GFX
JPEGDEC, https://github.com/bitbank2/JPEGDEC.git
NimBLE-Arduino, https://github.com/h2zero/NimBLE-Arduino
LISDHTR Acellerometer, https://travis-ci.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR.svg?branch=master
SparkFun_VL53L5CX_Arduino_Library, https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
Time, https://playground.arduino.cc/Code/Time/
TinyGPSPlus-ESP32, https://github.com/Tinyu-Zhao/TinyGPSPlus-ESP32
URLEncode, https://github.com/plageoj/urlencode
WiFiManager by Tzapu, https://github.com/tzapu/WiFiManager

Depends on Esspresif ESP32 libraries at
[https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)

Using Arduino IDE 2.x, use Tools menu options:
Board: ESP32S3 Dev Module
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
Partition Scheme: Defaults 4MB with Spiffs (1.2 MB APP/1.5MB SPIFFS)
PSRAM: Disabled
Upload Mode: UART0/Hardware CDC
Upload Speed 921600
USB Mode: Hardware CDC and JTAG
*/

#include "Arduino.h"
#include "BLE.h"
#include "Utils.h"
#include "config.h"
#include "Storage.h"
#include "Logger.h"
#include "Battery.h"
#include "Player.h"
#include "Video.h"
#include "MjpegClass.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include "secrets.h"
#include "Accellerometer.h"
#include "Audio.h"
#include "Compass.h"
#include "GPS.h"
#include "Gesture.h"
#include "Haptic.h"
#include "LED.h"
#include "USBFlashDrive.h"
#include "OTA.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/gpio.h"
#include "Hardware.h"
#include "Wire.h"

Utils utils;
Storage storage;
Player player;
Video video;

static Wifi wifi;
static GPS gps;
static Accellerometer accel;
static Compass compass;
static Gesture gesture;
static Haptic haptic;
Audio audio;
static LED led;
static USBFlashDrive flash;
static BLE ble;
static OTA ota;

const char *root_ca = ssl_cert;  // Shared instance of the server side SSL certificate, found in secrets.h

LOGGER logger;
Battery battery;
Hardware hardware;

// Host name
std::string devname;
String devicename;

// Timers

long locationUpdate;
long locationUpdateTimer;

#include <Arduino_GFX_Library.h>

/* Test for a device number on the I2C bus, display error when not found */

void assertI2Cdevice(int deviceNum, String devName) {
  Wire.beginTransmission(deviceNum);
  if (Wire.endTransmission() != 0) {
    video.stopOnError(devName, "not found", "", "", "");
  }
}

void assertTest(bool result, String deviceName) {
  if (!result) {
    video.stopOnError(deviceName, "test fail", "", "", "");
  }
}

/*
  Cooperative multi-tasking functions
*/

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    video.loop();
    audio.loop();
    player.loop();
    //logger.loop();
    battery.loop();

    /*
    gesture.loop();
    storage.loop();
    utils.loop();
    wifi.loop();
    accel.loop();
    compass.loop();
    haptic.loop();
    led.loop();
    ble.loop();
    */

  } while (millis() - start < ms);
}

void setup() {
  Serial.begin(115200);
  long time = millis();
  while (!Serial && (millis() < time + 5000))
    ;  // wait up to 5 seconds for Arduino Serial Monitor
  Serial.setDebugOutput(true);

  hardware.begin();   // Sets all the hardware pins

  Serial.println("- - -");
  Serial.println(F("Reflections of Frank"));
  Serial.println(" ");

  storage.begin();
  storage.setMounted( hardware.getMounted() );
  
  video.begin();
  player.begin();

  //wifi.reset();     // Optionally reset any previous connection settings
  wifi.begin();  // Non-blocking, until guest uses it to connect

  logger.begin();
  logger.setEchoToSerial(true);
  logger.setEchoToServer(true);

  logger.info(F("Reflections of Frank"));

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append(mac.substr(15, 2));
  devicename = devname.c_str();
  String hostinfo = "Host: ";
  hostinfo += devicename;
  logger.info(hostinfo);  

  locationUpdateTimer = millis();

  /* 
    Sometimes you need to delete all the files
    and try to load them again
  */

  if ( false ) {
    //Serial.println( "*1" );
    //storage.listDir(SD, "/", 100, true);    

    storage.replicateServerFiles();

    Serial.println( "**1" );
    storage.listDir(SD, "/", 100, true);
  }

  // Self-test: NAND, I2C, SPI
  // while playing startup animation and sound

  /*
  assertI2Cdevice(0x18, "Accelerometer");
  smartdelay(200);
  assertI2Cdevice(0x29, "Gesture");
  smartdelay(200);
  assertI2Cdevice(0x30, "Compass");
  smartdelay(200);
  assertI2Cdevice(0x5A, "Haptic");
  smartdelay(200);
  */

  haptic.begin();
  haptic.playEffect(14);  // 14 Strong Buzz

  battery.begin();
  audio.begin();

  //ota.begin();
  //ota.update();     // Just in case previous use of the host replicated an OTA update file

/*
  utils.begin();
  gps.begin();
  accel.begin();
  compass.begin();
  flash.begin();
  gesture.begin();
  led.begin();
  ble.begin();

  assertTest(storage.testNandStorage(), "NAND storage");
  smartdelay(200);
  assertTest(gps.test(), "NAND storage");
  smartdelay(200);
  assertTest(gps.test(), "GPS");
  smartdelay(200);
  assertTest(accel.test(), "Accelerometer");
  smartdelay(200);
  assertTest(compass.test(), "Compass");
  smartdelay(200);
  assertTest(gesture.test(), "Gesture");
  smartdelay(200);

  // startMSC();     // Calliope mounts as a flash drive, showing NAND contents over USB on your computer
*/

  logger.info(F("Setup complete"));
}

void loop() {
  smartdelay(10);

  // Send telemetry of sensors to Cloud City for analysis


  // Update BLE beacon heading for other devices

/*
  if ((millis() - locationUpdateTimer) > 2000) {
    locationUpdateTimer = millis();

    // Tell other devices the heading you are pointing towards
    float headfl = compass.getHeading();
    String myh = String(headfl);
    ble.setMessage(myh);
    ble.setLocalHeading(headfl);
  }
*/
}
