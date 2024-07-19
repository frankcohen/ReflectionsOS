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
NimBLE-Arduino, https://github.com/h2zero/NimBLE-Arduino
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
Partition Scheme: 8MB with Spiffs
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
#include "OTA.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/gpio.h"
#include "Hardware.h"
#include "Wire.h"
#include "Parallax.h"
#include "TimeService.h"
#include "Inveigle.h"
#include <Arduino_GFX_Library.h>

Utils utils;
Storage storage;
Video video;
Haptic haptic;
Audio audio;
TOF tof;
Parallax parallax;
TimeService timeservice;
Inveigle inveigle;
GPS gps;
Wifi wifi;
Compass compass;
LED led;
USBFlashDrive flash;
BLE ble;
OTA ota;
LOGGER logger;
Battery battery;
Hardware hardware;
Accelerometer accel;

const char *root_ca = ssl_cert;  // Shared instance of the server side SSL certificate, found in secrets.h

// Host name
std::string devname;
String devicename;

// Timers

long locationUpdate;        // For BLE service
long locationUpdateTimer;

unsigned long startvidtime;
bool startvidflag;
int startcnt;

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

  do {
    battery.loop();
    //accel.loop();
    tof.loop();
    video.loop();
    storage.loop();
    timeservice.loop();
    wifi.loop();  
    utils.loop();    
    inveigle.loop();
    compass.loop();
    haptic.loop();
    led.loop();
    //ble.loop();

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

  hardware.begin();   // Sets all the hardware pins

  Serial.println("- - -");
  Serial.println(F("Reflections of Frank"));
  Serial.println(" ");

  storage.begin();
  storage.setMounted( hardware.getMounted() );

  //utils.WireScan();

  video.begin();

  //wifi.reset();  // Optionally reset any previous connection settings
  //wifi.begin();  // Non-blocking, until guest uses it to connect

  logger.begin();
  logger.setEchoToSerial( true );
  logger.setEchoToServer( false );

  logger.info( F("Reflections of Frank") );

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append(mac.substr(15, 2));
  devicename = devname.c_str();
  String hostinfo = "Host: ";
  hostinfo += devicename;
  logger.info(hostinfo);  

  locationUpdateTimer = millis();
  
  // Self-test: NAND, I2C, SPI

  assertI2Cdevice(24, "Acclerometer");
  assertI2Cdevice(48, "Compass");
  assertI2Cdevice(90, "Haptic");
  assertI2Cdevice(41, "TOF Accel");

  //storage.replicateServerFiles();
  
  //Serial.println( "Files on board:" );
  //storage.listDir(SD, "/", 100, true);

  haptic.begin();
  battery.begin();
  audio.begin();
  gps.begin();
  accel.begin();
  compass.begin();
  tof.begin();
  parallax.begin();
  timeservice.begin();
  inveigle.begin();
  utils.begin();
  //ble.begin();
  led.begin();

  //accel.setTraining( true );    // Put accelermoeter into training mode
  //accel.loadGestures();         // Load the prerecorded accelermeter gestures

/*
  ota.begin();
  ota.update();     // Just in case previous use of the host replicated an OTA update file

  startMSC();     // Calliope mounts as a flash drive, showing NAND contents over USB on your computer
*/

  //inveigle.startExperience( Inveigle::SetTime );

  haptic.playEffect(14);  // 14 Strong Buzz

startvidtime = millis();
startvidflag = true;

  logger.info(F("Setup complete"));
}


const char* msgtext[5][2] = {
  { "It's early",    "why yes" },
  { "It's late",     "to be exact"},
  { "Wait, wait",    "you waited!"},
  { "Peekaboo",      "i see you"},
  { "Why?",          "why not?"},
};


void loop() {
  smartdelay(1000);

 if ( millis() - startvidtime > 500 )
 {
  startvidtime = millis();

  startcnt++;
  //Serial.println( startcnt );

  if ( startvidflag )
  {
    inveigle.startExperience( Inveigle::Awake );
    startvidflag = false;
  }

 }

  // Update BLE beacon heading for other devices

  if ( ble.isStarted() )
  {
    if ( (millis() - locationUpdateTimer) > 2000 )
    {
      locationUpdateTimer = millis();


      int index = random(0, 4);
      String theMsg1 = msgtext[ index ][ 0 ];
      String theMsg2 = msgtext[ index ][ 1 ];


      // Tell other devices the heading you are pointing towards
      float headfl = compass.getHeading();
      ble.setMessage( theMsg2 );
      ble.setLocalHeading(headfl);
    }
  }

}
