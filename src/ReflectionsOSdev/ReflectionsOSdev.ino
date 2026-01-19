/*
Reflections is a hardware and software platform for building entertaining mobile and wearable experiences.

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

/*
Reflections is a hardware and software platform for building entertaining mobile and wearable experiences.

... (header unchanged for brevity in this snippet) ...
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
ExperienceStats experiencestats(60000UL); // Create a global tracker with a 60 000 ms (1 min) reporting interval
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

bool tofstarted = false;
bool accelstarted = false;
bool blestarted = false;

// -----------------------------------------------------------------------------
// Experience pacing + time-setting gate
//   - 5 second cooldown between experiences (measured after an experience ENDS)
//   - Pounce ignores cooldown
//   - Sleep gesture / low battery / inactivity sleep bypasses cooldown (immediate)
//   - No experiences start while setting time (including Pounce and Sleep)
// -----------------------------------------------------------------------------

const unsigned long EXPERIENCE_COOLDOWN_MS = 5000; // 5 seconds between experiences (after one ends)
bool wasExperienceActive = false;
unsigned long lastExperienceEndedAt = 0;

bool cooldownGate()
{
  return (millis() - lastExperienceEndedAt) >= EXPERIENCE_COOLDOWN_MS;
}

// If wearer is setting time, do not start experiences (unless explicitly allowed)
bool canStartExperience(bool bypassCooldown = false, bool allowDuringTimeSetting = false)
{
  if (experienceservice.active()) return false;
  if (!allowDuringTimeSetting && watchfacemain.isSettingTime()) return false;
  if (!bypassCooldown && !cooldownGate()) return false;
  return true;
}

// -----------------------------------------------------------------------------
// Forward decls
// -----------------------------------------------------------------------------

void Core0Tasks(void *pvParameters);

// -----------------------------------------------------------------------------
// I2C assert
// -----------------------------------------------------------------------------

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

void BIUfailed( String text )
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

// Checks for low battery and goes to deep sleep

static void bootBatteryLowGate()
{
  if ( ! battery.isBatteryLow() ) return;

  // Match your prior Video::begin() behavior
  gfx->setFont(&Minya16pt7b);
  gfx->setTextSize(1);
  gfx->setCursor(45, 85);
  gfx->setTextColor(COLOR_TEXT_YELLOW);
  gfx->println(F("Battery low"));

  digitalWrite(Display_SPI_BK, LOW);  // backlight on (active low)

  Serial.printf( "Battery low at boot: now=%d mV\n", battery.getVoltageMv() );
  Serial.flush();

  delay(3000);

  // Go protect RTC immediately
  hardware.prepareForSleep();
  hardware.powerDownComponents();

  // Optional: if you want to preserve last-known clock even on low battery
  realtimeclock.saveClockToNVS();

  esp_deep_sleep_start();

  // Should never return
  while (true) { delay(1000); }
}

// Clears the NVS Flash memory
void clearNVSMemory()
{
  Serial.println( F("nvs_flash_init()") );
  nvs_flash_erase();
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
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

  if (SD.exists(mfd)) return;

  Serial.println(F("Board Initialization Utility started"));

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  printCentered("Initialize");

  // Start Wifi
  wifi.reset();
  printCentered(F("Wifi"));
  delay(1000);

  if (!wifi.begin())
  {
    BIUfailed("Wifi failed");
  }

  delay(1000);

  // NTP sync is handled by RealTimeClock again
  if (realtimeclock.syncWithNTP("pool.ntp.org", 5000))
  {
    Serial.println(F("Sync'd to network time over wifi"));
  }
  else
  {
    Serial.println(F("Could not sync to network time over wifi"));
    BIUfailed( F( "NTP failed" ) );
  }

  delay(1000);
  printCentered(F("Replicate"));
  delay(1000);

  storage.printStats();

  // Download cat-file-package.tar and any other files, then expand the tars
  if (!storage.replicateServerFiles())
  {
    BIUfailed(F("Replicate failed"));
  }

  Serial.println("After: ");
  storage.printStats();

  delay(1000);
  printCentered(F("Restart"));
  delay(1000);

  digitalWrite(Display_SPI_BK, HIGH);  // Turn display backlight off

  // Installation complete; enter Shipment Mode - draw essentially zero extra power in the box and not wake up from taps/motion
  hardware.enterShippingMode();

  // Should never return
  while (true) { delay(1000); }
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
    hardware.loop();    // Includes shipping mode shutdown

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

    // Debug messages on the display
    //String bmsg = battery.getDischargeOverlayText();
    //if (bmsg.length()) video.paintText(bmsg);    

  } while (millis() - start < ms);
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.setDebugOutput(true);

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  Serial.print("Wake cause: ");
  Serial.println((int)reason);

  if (reason == ESP_SLEEP_WAKEUP_EXT1) {
    hardware.prepareAfterWake();
    uint64_t st = esp_sleep_get_ext1_wakeup_status();
    Serial.print("EXT1 wake mask: 0x");
    Serial.println((uint32_t)st, HEX);   // or (unsigned long long)st if you prefer
  }
  else
  {
    Serial.println(F(" "));
    Serial.println(F("Starting"));
    Serial.println(F("ReflectionsOS"));
  }

  systemload.begin();
  systemload.printHeapSpace( "Start" );

  hardware.begin();
  haptic.begin();
  haptic.playEffect(14);  // 14 Strong Buzz

  battery.begin();
  video.begin();          // needed so we can show "Battery low" reliably
  video.setPaused( true );

  bootBatteryLowGate();   // single battery-low policy point for ALL wakes

  // Core 1 services

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

  BoardInitializationUtility();

  mjpegrunner.begin();
  systemload.printHeapSpace( "Video" );

  // Device initialization
  audio.begin();
  gps.begin();
  compass.begin();
  utils.begin();
  realtimeclock.begin();

  systemload.printHeapSpace( F("Devices") );

  // Support service initialization
  steps.begin();
  timerservice.begin();

  // Core 0 services
  tofstarted = false;
  accelstarted = false;
  blestarted = false;

  xTaskCreatePinnedToCore(
    Core0Tasks,
    "Core0Tasks",
    32768,
    NULL,
    1,
    NULL,
    0
  );

  // Experience initialization
  textmessageservice.begin();
  experienceservice.begin();
  watchfaceexperiences.begin();
  watchfacemain.begin();

  // Start first experience immediately at boot (no cooldown)
  experienceservice.startExperience( ExperienceService::Awake );

  // Initialize pacing state so we don't artificially delay after boot
  wasExperienceActive = experienceservice.active();
  lastExperienceEndedAt = millis() - EXPERIENCE_COOLDOWN_MS;

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

  pinMode(0, INPUT_PULLUP);

  logger.info(F("Setup complete"));
}

/* Runs TOF and Accelerometer gesture sensors in core 0 */
void Core0Tasks(void *pvParameters) {

  while (true) {
    if ( ! tofstarted)
    {
      tof.begin();
      tofstarted = true;
    }
    else
    {
      tof.loop();
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

    vTaskDelay(pdMS_TO_TICKS(10));
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

unsigned long catTimer = millis();

void loop()
{
  printCore0TasksMessages();

  // Track transition: active -> stopped, so we can pace the next start
  bool nowActive = experienceservice.active();
  if (wasExperienceActive && !nowActive)
  {
    lastExperienceEndedAt = millis(); // cooldown starts when an experience ends
  }
  wasExperienceActive = nowActive;

  // If an experience is active, do nothing else
  if ( experienceservice.active() )
  {
    smartdelay(10);
    return;
  }

  // Pounce message received — ignores cooldown (but still blocked while setting time)
  if ( ( blesupport.isPounced() ) && ( millis() - pounceTimer > 10000 ))
  {
    if (!canStartExperience(true, false)) { smartdelay(10); return; } // bypassCooldown=true

    pounceTimer = millis();
    Serial.println( "Pounce from an other device" );
    textmessageservice.deactivate();
    experienceservice.startExperience( ExperienceService::Pounce );
    waitForExperienceToStop();
    Serial.println( "Pounce done" );
    smartdelay(10);
    return;
  }

  // Sleepy after minutes of WatchFaceMain in MAIN and no activity
  if ( watchfacemain.isSleepy() )
  {
    if (!canStartExperience(false, false)) { smartdelay(10); return; }

    Serial.println("Getting sleepy");
    textmessageservice.stop();
    experienceservice.startExperience( ExperienceService::Sleep );
    smartdelay(10);
    return;
  }

  // There's another cat nearby!
  if ( ( blesupport.isCatNearby() > 0 ) &&
       ( millis() - catTimer > ( 60000 * 3 ) ) &&
       ( ! watchfacemain.isSettingTime() ) )
  {
    if (!canStartExperience(false, false)) { smartdelay(10); return; }

    catTimer = millis();
    Serial.println( "Cats Play" );
    textmessageservice.deactivate();
    experienceservice.startExperience( ExperienceService::CatsPlay );
    smartdelay(10);
    return;
  }

  int recentGesture = tof.getGesture();

  if (recentGesture != GESTURE_NONE)
  {
    Serial.print("Gesture: ");

/*
      experienceservice.startExperience( ExperienceService::Pensive );
      smartdelay(10);
      return;
*/

    switch (recentGesture)
    {
      case GESTURE_LEFT_RIGHT:
        Serial.println(">>>GESTURE_LEFT_RIGHT");
        break;

      case GESTURE_RIGHT_LEFT:
        Serial.println(">>>GESTURE_RIGHT_LEFT");
        break;

      case GESTURE_CIRCULAR:
        Serial.println(">>>GESTURE_CIRCULAR");
        break;

      case GESTURE_SLEEP:
        Serial.println(">>>GESTURE_SLEEP");
        break;

      default:
        Serial.printf(">>>Unknown (%d)\n", recentGesture);
        break;
    }
  }

  battery.setSleepCountdownMs( watchfacemain.getSleepCountdown() );

  // Go to sleep when gestured or when the battery is low or inactivity says so
  if ( ( recentGesture == GESTURE_SLEEP ) || battery.isBatteryLow() || watchfacemain.goToSleep() )
  {
    // Sleep should happen immediately: bypass cooldown.
    // Still blocked while setting time (your preference).
    if (!canStartExperience(true, false)) { smartdelay(10); return; }

    if ( recentGesture == GESTURE_SLEEP ) Serial.println("Going to sleep for gesture");
    if ( battery.isBatteryLow() ) Serial.println("Going to sleep for low battery");
    if ( watchfacemain.goToSleep() ) Serial.println("Going to sleep for inactivity");

    textmessageservice.stop();
    experienceservice.startExperience(ExperienceService::Sleep);
    waitForExperienceToStop();

    // Configure LIS3DH wake profile while system is fully "awake" (I2C stable)
    accel.configureWakeTapProfile();
    delay(20); // let INT settle

    // Now shut down the rest + enable holds
    hardware.prepareForSleep();

    Serial.println("Entering deep sleep now");
    Serial.flush();
    realtimeclock.saveClockToNVS();
    esp_deep_sleep_start();
    return;
  }

  // From here onward: do not start ANY non-sleep experiences while setting time
  if ( watchfacemain.isSettingTime() )
  {
    smartdelay(10);
    return;
  }

  if ( accel.isShaken() )
  {
    if (!canStartExperience(false, false)) { smartdelay(10); return; }

    experienceservice.startExperience( ExperienceService::Shaken );
    smartdelay(10);
    return;
  }

  // No new gestures (except for sleep) unless watchface is on MAIN
  if ( ! watchfacemain.isMainOrTime() )
  {
    smartdelay(10);
    return;
  }

  if ( recentGesture == GESTURE_RIGHT_LEFT || recentGesture == GESTURE_CIRCULAR || recentGesture == GESTURE_LEFT_RIGHT )
  {
    if ( experiencestats.isFrank() )
    {
      if (!canStartExperience(false, false)) { smartdelay(10); return; }
      experienceservice.startExperience( ExperienceService::EasterEggFrank );
      smartdelay(10);
      return;
    }

    if ( experiencestats.isTerri() )
    {
      if (!canStartExperience(false, false)) { smartdelay(10); return; }
      experienceservice.startExperience( ExperienceService::EasterEggTerri );
      smartdelay(10);
      return;
    }
  }

  if ( recentGesture == GESTURE_RIGHT_LEFT )
  {
    if (!canStartExperience(false, false)) { smartdelay(10); return; }
    
    //experienceservice.startExperience( ExperienceService::MysticCat );
    
    experienceservice.startExperience( ExperienceService::Pensive );
    smartdelay(10);
    return;
  }

  // Left to Right gives all experiences one-at-a-time
  if ( recentGesture == GESTURE_LEFT_RIGHT )
  {
    if (!canStartExperience(false, false)) { smartdelay(10); return; }

      experienceservice.startExperience( ExperienceService::Pensive );
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
    if (!canStartExperience(false, false)) { smartdelay(10); return; }

    if ( nextUp2 == 0 )
    {
      nextUp2 = 1;
      experienceservice.startExperience( ExperienceService::EyesFollowFinger );
      smartdelay(10);
      return;
    }

    if ( nextUp2 == 1 )
    {
      nextUp2 = 2;
      experienceservice.startExperience( ExperienceService::ParallaxCat );
      smartdelay(10);
      return;
    }

    if ( nextUp2 == 2 )
    {
      nextUp2 = 0;
      experienceservice.startExperience( ExperienceService::Sand );
      smartdelay(10);
      return;
    }
  }

  smartdelay(10);
}
