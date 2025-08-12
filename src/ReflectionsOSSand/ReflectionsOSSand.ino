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

Adafruit DRV2605 Library, haptic controller, https://github.com/adafruit/Adafruit_DRV2605_Library
Adafruit LIS3DH library, accelerometer, https://github.com/adafruit/Adafruit_LIS3DH
Adafruit MMC56x3, compass, magnetometer, https://github.com/adafruit/Adafruit_MMC56x3
The Adafruit I2C libraries, like MMC56x3, depend on these
Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
ArduinoJson, https://arduinojson.org/
ESP32-targz, https://github.com/tobozo/ESP32-targz/
ESP8266Audio, https://github.com/earlephilhower/ESP8266Audio
HTTPClient, https://github.com/amcewen/HttpClient
GFX Library for Arduino, https://github.com/moononournation/Arduino_GFX
SparkFun_VL53L5CX_Arduino_Library, TOF sensor, https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
NimBLE stack, https://github.com/h2zero/NimBLE-Arduino
NTPClient, Network Time Protocol, https://github.com/arduino-libraries/NTPClient
ESP32 JPEG Library, https://github.com/bitbank2/JPEGDEC.git
PNGdec library, https://github.com/bitbank2/PNGdec
TinyGPSPlus-ESP32, https://github.com/Tinyu-Zhao/TinyGPSPlus-ESP32
ESP32Time library from https://github.com/fbiego/ESP32Time

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
Partition Scheme: Custom, Reflections App (8MB, No OTA No SPIFFS)    See below
PSRAM: Disabled ( ESP32-S3-MINI-1-N8 has no PSRAM )
Upload Mode: UART0/Hardware CDC
Upload Speed 921600
USB Mode: Hardware CDC and JTAG
Zigbee Mode: Disabled

Reflections uses a custom partition scheme to maximize available flash storage.
See instructions at: https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/Partition%20tables%20and%20optimizing%20memory%20in%20Arduino%20IDE.md
Arduino IDE 2.x automatically uses partitions.csv in the source code directory

Sometimes you may need to clear the ESP32-S3 flash memory, use this command:
/Users/frankcohen/Library/Arduino15/packages/esp32/tools/esptool_py/4.6/esptool" --chip esp32s3 --port "/dev/cu.usbmodem1101" --baud 921600 erase_flash

same for clearing cached compiler data on MacOS:
cd /Users/frankcohen/Library/Caches/arduino/sketches

Reflections reuses the ESP32S3 Dev Module board definition provided by Espressif:
~/Library/Arduino15/packages/esp32/hardware/esp32/3.2.0/boards.txt
in this section: esp32s3.name=ESP32S3 Dev Module

*/

#include "Arduino.h"
#include "BLEsupport.h"
#include "Utils.h"
#include "config.h"
#include "Storage.h"
#include "Logger.h"
#include "Battery.h"
#include "MjpegRunner.h"
#include "Video.h"
#include "secrets.h"
#include "AccelSensor.h"
#include "Audio.h"
#include "Compass.h"
#include "GPS.h"
#include "TOF.h"
#include "Haptic.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "Hardware.h"
#include "Wire.h"
#include "TextMessageService.h"
#include <Arduino_GFX_Library.h>
#include "WatchFaceExperiences.h"
#include "WatchFaceMain.h"
#include "nvs_flash.h"
#include "RealTimeClock.h"
#include "Steps.h"
#include "TimerService.h"
#include "SystemLoad.h"
#include "ScienceFair14pt7b.h"
#include "ExperienceStats.h"
#include "ExperienceService.h"

MjpegRunner mjpegrunner;
Video video;
Utils utils;
Storage storage;
Haptic haptic;
Audio audio;
TOF tof;
TextMessageService textmessageservice;
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
BLEsupport blesupport;
ExperienceStats experiencestats(60000UL); // Create a global tracker with a 60 000 ms (1 min) reporting interval
ExperienceService experienceservice;

int rowCount = 0;
unsigned long statstime = millis();
unsigned long pounceTimer = millis();
unsigned long catNearBy = millis();
unsigned long afterTimer = millis();
unsigned long gestureTimer = millis();
unsigned long afterCatsPlay = millis();

int nextUp = 0;     // For picking the next experience from a left-to-right gesture
int nextUp2 = 0;    // For picking between Eyes and Parallax experiences

const char *root_ca = ssl_cert;  // Shared instance of the server side SSL certificate, found in secrets.h

bool tofstarted = false;
bool accelstarted = false;
bool blestarted = false;

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

  video.stopOnError(devName, F("not found"), "", "", "");
}

int16_t in_x, in_y;
uint16_t in_w, in_h;

void printCentered( String text )
{
  gfx->fillScreen( COLOR_BLUE );
  gfx->setFont( &ScienceFair14pt7b );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->getTextBounds( text.c_str(), 0, 0, &in_x, &in_y, &in_w, &in_h);

  in_y = 120;
  in_x = 40;

  gfx->setCursor( ( gfx->width() - in_w ) / 2, in_y );
  gfx->println( text );

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
}

void BIUfaled( String text )
{
  gfx->fillScreen( COLOR_RED );
  gfx->setFont( &ScienceFair14pt7b );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->getTextBounds( text.c_str(), 0, 0, &in_x, &in_y, &in_w, &in_h);

  in_y = 120;
  in_x = 40;

  gfx->setCursor( ( gfx->width() - in_w ) / 2, in_y );
  gfx->println( text );

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  Serial.println(F("Board Initialization Utility Failed "));
  Serial.println( text );
  Serial.println( F("Stopping") );
  while (1);
}

// Clears the NVS Flash memory

void clearNVSMemory()
{
  Serial.println( F("nvs_flash_init()") );
  nvs_flash_erase();
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated and needs to be erased
      Serial.println( F("nvs_flash_erase()") );
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  Serial.println( F("nvs done") );
}

/*
  Reflections board initialization utility
  Mounts the SD, connects to Wifi, replicates the disk image tar file,
  unpacks the tar, restarts
*/

void BoardInitializationUtility()
{
  // Check if otaversion.txt exists, if so skip initialization

  String mfd = F("/");
  mfd += NAND_BASE_DIR;
  mfd += OTA_VERSION_FILE_NAME;

  if ( SD.exists( mfd ) ) return;
  
  Serial.println( F( "Board Initialization Utility started" ) );
  
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  printCentered("Initialize");

  // Start Wifi

  wifi.reset();  // Optionally reset any previous connection settings
  printCentered( F( "Wifi" ) );
  delay( 1000 );

  if ( ! wifi.begin() )  // Non-blocking, until guest uses it to connect
  {
    BIUfaled( "Wifi failed" );
  }

  delay( 1000 );
  if ( realtimeclock.syncWithNTP("pool.ntp.org", 5000) )
  {
    Serial.println( F( "Sync'd to network time over wifi" ) );
  }
  else
  {
    Serial.println( F( "Could not sync to network time over wifi" ) );
  }

  delay( 1000 );
  printCentered( F( "Replicate" ) );
  delay( 1000 );

  storage.printStats();

  // Download cat-file-package.tar and any other files, then expand the tars

  if ( ! storage.replicateServerFiles() )
  {
    BIUfaled( F("Replicate failed") );
  }

  Serial.println( "After: ");
  storage.printStats();

  /*
  Serial.println( F("- - -") );
  Serial.println( F("Files:") );
  storage.listDir(SD, F("/"), 100, true);
  */

  delay( 1000 );
  printCentered( F( "Restart" ) );
  delay( 1000 );

  digitalWrite(Display_SPI_BK, HIGH);  // Turn display backlight off

  ESP.restart();
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
    video.loop();
    battery.loop();
    storage.loop();
    wifi.loop();
    utils.loop();
    compass.loop();
    haptic.loop();
    realtimeclock.loop();
    gps.loop();
    steps.loop();
    timerservice.loop();
    audio.loop();

    // Experience operations

    unsigned long fellow = millis();

    if ( experienceservice.getCurrentState() == ExperienceService::STOPPED )
    {  
      watchfaceexperiences.loop();
      systemload.logtasktime(millis() - fellow, 1, "we");
    }
    else
    {
      fellow = millis();
      experienceservice.loop();
      systemload.logtasktime(millis() - fellow, 2, F("ex"));
    }

    fellow = millis();
    textmessageservice.loop();
    systemload.logtasktime(millis() - fellow, 3, "tm");

  	systemload.loop();

    systemload.logtasktime( millis() - tasktime, 0, " " );

    // Paint debug details over display
    //video.paintText( battery.getBatteryStats() );
    //video.paintText( gps.getStats() );

  } while (millis() - start < ms);
}

void setup() 
{
  Serial.begin(115200);
  delay(200);
  Serial.setDebugOutput(true);

  //utils.WireScan();   // Shows devices on the I2S bus, including compass, TOF, accelerometeer
  //clearNVSMemory()    // Clear the non-volitile storage

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  if (reason == ESP_SLEEP_WAKEUP_EXT1) 
  {
    Serial.println("Waking from deep sleep");
  } 
  else 
  {
    Serial.println(F(" "));
    Serial.println(F("Starting"));
    Serial.println(F("ReflectionsOS"));
  }

  systemload.begin();  // System load monitor
  systemload.printHeapSpace( "Start" );

  // Doing this here to get some color onto the display quickly
  // for the usere to think the startup is quick

  hardware.begin();  // Sets all the hardware pins
  haptic.begin();
  haptic.playEffect(14);  // 14 Strong Buzz
  video.begin();
  video.setPaused( true );

  // Core 1 services

  mjpegrunner.begin();
  systemload.printHeapSpace( "Video" );

  storage.begin();
  storage.setMounted( hardware.getMounted() );

  logger.begin();
  logger.setEchoToSerial(true);
  logger.setEchoToServer(false);

  String hostinfo = F("Host: ");
  hostinfo += wifi.getDeviceName().c_str();
  hostinfo += F(", ");
  hostinfo += wifi.getMACAddress();
  logger.info(hostinfo);

  // Self-test: NAND, I2C, SPI

  assertI2Cdevice(24, "Acclerometer");
  assertI2Cdevice(48, "Compass");
  assertI2Cdevice(90, "Haptic");
  assertI2Cdevice(41, "TOF Accel");

  // Device initialization

  battery.begin();
  audio.begin();
  gps.begin();
  compass.begin();
  utils.begin();
  realtimeclock.begin();

  systemload.printHeapSpace( F("Devices") );

  BoardInitializationUtility();   // Installs needed video and other files

  // Support service initialization

  steps.begin();
  timerservice.begin();

  // Core 0 services

  tofstarted = false;
  accelstarted = false;
  blestarted = false;

  // Create a new task for TOF processing, pin it to core 0
  xTaskCreatePinnedToCore(
    Core0Tasks,    // Task function
    "Core0Tasks",  // Name of the task
    32768,         // Stack size (in words, not bytes)
    NULL,          // Task input parameter
    1,             // Priority of the task
    NULL,          // Task handle
    0              // Core where the task should run (core 0)
  );
  
  // Experience initialization

  textmessageservice.begin();
  experienceservice.begin();
  watchfaceexperiences.begin();
  watchfacemain.begin();

  experienceservice.startExperience( ExperienceService::Awake );

  systemload.printHeapSpace( "Experience services" );

  while ( ! tofstarted )
  {
    smartdelay(1000);
  }

  video.setPaused( false );
  tof.setStatus( true );
  accel.setStatus( true );
  gps.on();

  systemload.printHeapSpace( "Setup done" );

  logger.info(F("Setup complete"));
}

/* Runs TOF and Accelerometer gesture sensors in core 0 */

void Core0Tasks(void *pvParameters) {

  int muck = 0;

  while (true) {
    if ( ! tofstarted) 
    {
      tof.begin();
      tofstarted = true;
    } 
    else 
    {
      tof.loop(); // Process and update gesture data
    }

    if (!accelstarted) {
      accel.begin();
      accelstarted = true;
    } else {
      accel.loop();
    }

    if ( ! blestarted )
    {
      blesupport.begin();
      systemload.printHeapSpace( "BLE" );
      blestarted = true;
    }
    else
    {
      blesupport.loop();
    }

    // Delay to prevent task from monopolizing the CPU
    vTaskDelay(pdMS_TO_TICKS(10));  // Delay for 10 milliseconds
  }
}

// Printing accelerometer and TOF messages here because this 
// code runs in Core 1 otherwise the messages compete for the Serial monitor

void printCore0TasksMessages()
{
  if ( millis() - statstime > 1500 )
  {
    statstime = millis();

    String mf = tof.getRecentMessage();
    String mf2 = tof.getRecentMessage2();

    if ( mf != "" ) 
    {
      Serial.println( mf );
      Serial.println( mf2 );
    }

    mf = accel.getRecentMessage();
    mf2 = accel.getRecentMessage2();

    if ( mf != "" ) 
    {
      Serial.println( mf );
      Serial.println( mf2 );
    }
  }
}

void waitForExperienceToStop()
{
  while ( experienceservice.active() )
  {
    smartdelay(10);
  }
}

/*
  Main loop for controlling experiences and main watch face
*/

bool pouncetest = true;
unsigned long catTimer = millis();

void loop() 
{
  printCore0TasksMessages();  // Messages coming from TOF and Accelerometer services

  // Pounce message received
  if ( ( blesupport.isPounced() ) && ( millis() - pounceTimer > 10000 ))
  {
    pounceTimer = millis();
    Serial.println( "Pounce from an other device" );
    textmessageservice.deactivate();
    experienceservice.startExperience( ExperienceService::Pounce );
    waitForExperienceToStop();
    Serial.println( "Pounce done" );
    // Put cat into deep sleep  
    //hardware.powerDownComponents();
    //esp_deep_sleep_start();
    return;
  }

  if ( experienceservice.active() )
  {
    smartdelay(10);
    return;
  }
  
  // Sleepy after minutes of WatchFaceMain in MAIN and no activity

  if ( watchfacemain.isSleepy() )
  {
    Serial.println("Getting sleepy");
    experienceservice.startExperience( ExperienceService::Sleep );
    smartdelay(10);
    return;
  }

  // There's another cat nearby!
  if ( ( blesupport.isCatNearby() > 0 ) && ( millis() - catTimer > 60000 ) )
  {
    catTimer = millis();
    Serial.println( "Cats Play" );
    textmessageservice.deactivate();
    experienceservice.startExperience( ExperienceService::CatsPlay );
    smartdelay(10);
    return;   
  }
  
  int recentGesture = tof.getGesture();

  if ( recentGesture != GESTURE_NONE )
  {
    Serial.print( "Gesture: " );
    if ( recentGesture == GESTURE_LEFT_RIGHT ) Serial.println( ">>>GESTURE_LEFT_RIGHT" );
    if ( recentGesture == GESTURE_RIGHT_LEFT ) Serial.println( ">>>GESTURE_RIGHT_LEFT" );
    if ( recentGesture == GESTURE_CIRCULAR ) Serial.println( ">>>GESTURE_CIRCULAR" );
    if ( recentGesture == GESTURE_SLEEP ) { Serial.println( ">>>GESTURE_SLEEP" ); }
    if ( recentGesture == GESTURE_NONE ) { smartdelay(10); return; }
    else { Serial.println( ">>>Unknown" ); }
  }

 // Go to sleep when gestured or when the battery is low

  if ( ( recentGesture == GESTURE_SLEEP ) || battery.isBatteryLow() || watchfacemain.goToSleep() )
  {
    Serial.println("Going to sleep for gesture or low battery");
    experienceservice.startExperience( ExperienceService::Sleep );
    waitForExperienceToStop();
    // Put cat into deep sleep  
    hardware.powerDownComponents();
    esp_deep_sleep_start();
    return;
  }

  if ( accel.isShaken() )
  {
    experienceservice.startExperience( ExperienceService::Shaken );
    smartdelay(10);
    return;
  }

  // Wait 15 seconds after an experience before doing another experience
  if ( ! experienceservice.timeToRunAnother() )
  {
    smartdelay(10);
    return;
  }

  // No new gestures (except for sleep) unless watchface is on MAIN
  if ( ! watchfacemain.isMain() ) 
  {
    smartdelay(10);
    return;   
  }
  
  if ( recentGesture == GESTURE_RIGHT_LEFT || recentGesture == GESTURE_CIRCULAR || recentGesture == GESTURE_LEFT_RIGHT )
  {
    if ( experiencestats.isFrank() )
    {
      experienceservice.startExperience( ExperienceService::EasterEggFrank );
      smartdelay(10);
      return;
    }

    if ( experiencestats.isTerri() )
    {
      experienceservice.startExperience( ExperienceService::EasterEggTerri );
      smartdelay(10);
      return;
    }
  }

  if ( recentGesture == GESTURE_RIGHT_LEFT )
  {  
    experienceservice.startExperience( ExperienceService::Sand );
    //experienceservice.startExperience( ExperienceService::MysticCat );
    smartdelay(10);
    return;
  } 
  
  // Left to Right gives all experiences one-at-a-time

  if ( recentGesture == GESTURE_LEFT_RIGHT )
  {

    experienceservice.startExperience( ExperienceService::Sand );
    smartdelay(10);
    return;


    if ( nextUp == 0 )
    {
      nextUp++;
      experienceservice.startExperience( ExperienceService::Hover );
      smartdelay(10);
      return;
    }
    if ( nextUp == 1 )
    {
      nextUp++;
      experienceservice.startExperience( ExperienceService::EyesFollowFinger );
      smartdelay(10);
      return;
    }
    if ( nextUp == 2 )
    {
      nextUp++;
      experienceservice.startExperience( ExperienceService::Chastise );    
      smartdelay(10);
      return;
    }

    if ( nextUp == 3 )
    {
      nextUp++;
      experienceservice.startExperience( ExperienceService::Shaken );
      smartdelay(10);
      return;
    }

    if ( nextUp == 4 )
    {
      nextUp++;
      experienceservice.startExperience( ExperienceService::ParallaxCat );
      smartdelay(10);
      return;
    }

    if ( nextUp == 5 )
    {
      nextUp = 0;
      experienceservice.startExperience( ExperienceService::ShowTime );
      smartdelay(10);
      return;
    }

  }

  if ( recentGesture == GESTURE_CIRCULAR )
  {
    experienceservice.startExperience( ExperienceService::Sand );
    smartdelay(10);
    return;


    if ( nextUp2 == 1 )
    {
      nextUp2 = 0;
      experienceservice.startExperience( ExperienceService::EyesFollowFinger );
      return;
    }
    else
    {
      nextUp2 = 1;
      experienceservice.startExperience( ExperienceService::ParallaxCat );
      return;
    }

  };

  smartdelay(10);
}
