/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

*/

#ifndef _config_
#define _config_

#include "NimBLEDevice.h"

/* Experience videos */

#define OutOfTheBox_video "a1d7900fc81c714ccbdb692231c16d70"

#define Chastise_video "29eebc0d0804260f934575ceaedc50b1"

#define CatsPlayFound_video "5a9d9f72264522abc51300f9e772b136"
#define Pounce_video "c3d2eb0ad90b7a91db981fe01e8ee1b3"

#define CatsPlay1_video "001c0e74e770ec34c053879be10d1a7e"    // E
#define CatsPlay2_video "6216cf1923de998474b25afeda659448"    // NE
#define CatsPlay3_video "d5794454c11bca1f990e6a817e9b3b1e"    // N
#define CatsPlay4_video "6f119daa4433a28d44e19f37c4d9133f"    // NW
#define CatsPlay5_video "22ccc18e5b0c467fa209af54940b6773"    // W
#define CatsPlay6_video "98d5dacbc40ef6b1b8cfec4e5c6d230c"    // SW
#define CatsPlay7_video "bb0f5dbf32a9000787626cf71057fa26"    // S
#define CatsPlay8_video "6f19334918db04ddf84050696485c0d9"    // SE

#define EyesFollowFinger_video "f99c9b5031e6102ef45fbdf71cb09134"

#define MysticCat_video "af8d13e501bc2e6d61275e5faffde141"

#define ParallaxCat_video "7f4f62c859d8a8638cb9126c23775372"

#define Shaken_video "538b9f5360a511bf20e1b85a1dc6250d"

#define Sleep_video "c13cccc1f14b431659a70127aa276452"

#define Swipe_video "4e93a6c65511b257e959b0cdc9f9709f"

#define ShowTime_video "0c694842005c6e03d2abe845e530815e"

#define SetTime_video "c0fad0b76200936d73785cfc220d22cd"

#define Getting_Sleepy_video "69e2f5a6b7bfffab4a787be11f57fbcd"

#define videoname_end ".mjpeg"

// WatchFace_Main
#define wfMainBackground "Face_Main_background_baseline.jpg"
#define wfMainBattery "Battery_"
#define wfMainBattery2 ".png"
#define wfMainHours "Hours_"
#define wfMainHours2 ".png"
#define wfMainMinutes "Minutes_"
#define wfMainMinutes2 ".png"
#define wfMainFace "Blink_"
#define wfMainFace2 ".png"
#define wfMainFaceBlue "Blink_Blue2Blue_"
#define wfMainFaceBlue2 ".png"
#define blinkspeed 100
#define wfMainFlip "Main_Flip"
#define wfMainFlip2 ".png"
#define wfMainMaxFlips 14
#define flipspeed 50
#define wfMainHourglass "Main_HourGlass"
#define wfMainHourglass2 ".png"
#define WatchFaceOpener_video "809090d2b1c7b3829a3a72f583d58a7b"
#define WatchFaceFlip1_video "fb4bb116c31cd13777604f0c9c15f649"
#define WatchFaceFlip2_video "993d7b88b5b61b5488ad00bfd8d0c990"
#define WatchFaceFlip3_video "a4a5922d89d70c079514a5a21648804a"
#define wfMain_Timer_Background "Face_Main_Timer_Background.jpg"
#define wfMain_SetTimer_Background "Face_Main_SetTimer_Background.jpg"
#define wfMain_SetTimer_Background_Shortie "Face_Main_SetTimer_Background_Shortie.jpg"
#define wfMain_Health_Background "Face_Main_Health_Background2.jpg"
#define wfMain_SetTime_Background_Hour "Face_Main_SetTime_Hour_Background3.jpg"
#define wfMain_SetTime_Background_Minute "Face_Main_SetTime_Minute_Background3.jpg"
#define wfMain_SetTime_Background_Hour_Shortie "Face_Main_SetTime_Hour_Background3_Shortie.jpg"
#define wfMain_SetTime_Background_Minute_Shortie "Face_Main_SetTime_Minute_Background3_Shortie.jpg"
#define wfMain_Time_Background "Face_Main_Time_Background.jpg"
#define wfMain_Time_Background_Shortie "Face_Main_Time_Background_Shortie.jpg"
#define wfMain_Face_Main_TimerNotice "Face_Main_TimerNotice.png"
#define wfMain_Face_BlueDot_Background "Face_Main_Blue_Dot_Background.jpg"
#define wfMain_GPSmarker "GPSmarker.png"
#define wfMain_BackgroundBlue RGB565( 55, 160, 223 )

#define TRANSPARENT_COLOR 0xFFE0
#define TRANSPARENT_COLOR_PNG 0x00000000

// Host identification
#define host_name_me "REFLECTIONS-"

// OTA uses this to know when to update the firmware
#define VERSION_NUMBER 2
#define OTA_VERSION_FILE_NAME "/otaversion.txt"
#define OTA_BIN_FILE_NAME "/ota.bin"

#define MJPEG_FILENAME ".mjpeg"
#define TAR_FILENAME ".tar"
#define JSON_FILENAME ".json"

// Display
#define Display_SPI_DC    5
#define Display_SPI_CS    12
#define Display_SPI_RST   0
#define Display_SPI_BK    6

// SPI Bus
#define SPI_MOSI      35
#define SPI_MISO      37
#define SPI_SCK       36
#define SPI_SPEED     10000000

// NAND Storage
#define NAND_SPI_CS   15
#define NAND_SPI_PWR  11
#define NAND_BASE_DIR "REFLECTIONS"
#define LOGNAME_START "/REFLECTIONS/log"
#define LOGNAME_END ".txt"

// Accelerometer template storage
#define ACCEL_BASE_DIR "REFLECTIONS"
#define ACCEL_BASE_FILE "fantastic4"
#define ACCEL_BASE_EXT ".ages"

// Accellerometer
#define ACCEL_INT1_PIN GPIO_NUM_14
#define ACCEL_INT2_PIN GPIO_NUM_13
#define accelAddress 0x18     // 0x19 for alternative i2c address

// GPS
#define RXPin         18
#define TXPin         17
#define GPSPower      21

// Real Time Clock (RTC)
#define timeRegionOffset -1

// Gesture Time Of Flight (TOF)
#define TOFPower  26

// Audio
#define I2S_bclkPinP       9
#define I2S_wclkPinP      10
#define I2S_doutPinP       8
#define AudioPower         7

// I2C
#define I2CSDA        3
#define I2CSCL        4

// BLE

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

static const NimBLEUUID BLE_SERVICE_UUID( "7d9029fe-48d5-49e0-b9ad-8fd7dac70354" );
static const NimBLEUUID BLE_CHARACTERISTIC_UUID( "33a03886-0db0-43c8-90a2-bca6a8c7eaf0" );

#define bleServerName "REFLECTIONS"

// LED

#define LED_Count 12
#define LED_Type NEOPIXEL
#define LED_Pin 39

// TOF Gesture Sensing

#define tof_detectionThreshold 1
#define tof_pollingInterval 1000
#define tof_majorityThreshold 1
#define tof_expiration 2000

// Battery

#define Battery_Sensor 16

/*
 The earth's magnetic field varies according to its location.
 Add or subtract a constant to get the right value
 of the magnetic field using the following site
 http://www.ngdc.noaa.gov/geomag-web/#declination
*/
#define DECLINATION -0.08387 // declination (in degrees) in Silicon Valley, California USA

// Cloud City REST services

// CloudCityURL defined in secrets.h

// Gets json format of available files
#define cloudCityListFiles "https://cloudcity.starlingwatch.com/api/listfiles"

// Recorder UX
#define cloudCityListRecorder "https://cloudcity.starlingwatch.com/"

// Show list of files in html view - with functions to download and delete
#define cloudCityFiles "https://cloudcity.starlingwatch.com/api/files"

// Logger URL
#define cloudCityLogURL "https://cloudcity.starlingwatch.com/api/logit?message="

// Logger POST URL
#define cloudCityLogPostURL "https://cloudcity.starlingwatch.com/api/logitpost"

/* Show Time text and colors */

#define COLOR_BACKGROUND RGB565(115, 58, 0)
#define COLOR_LEADING RGB565(123, 63, 0)
#define COLOR_RING RGB565(234, 68, 0)
#define COLOR_TRAILING RGB565(178, 67, 0 )
#define COLOR_TEXT RGB565( 234, 67, 0 )
#define COLOR_TEXT_BACKGROUND RGB565( 79, 42, 0)
#define COLOR_TEXT_BORDER RGB565( 207, 67, 0 )

#define COLOR_TEXT_YELLOW RGB565( 210, 200, 102 )
#define COLOR_STRIPE_MEDIUM_GRAY RGB565( 153, 105, 140 )
#define COLOR_STRIPE_PINK RGB565( 166, 139, 186 )
#define COLOR_BLUE RGB565( 0, 0, 140 )
#define COLOR_RED RGB565( 140, 0, 0 )
#define COLOR_BLACK RGB565(0, 0, 0)
#define COLOR_MAIN_BACK RGB565( 55, 161, 224 )

#define COLOR_EYES_LEFT RGB565( 192, 169, 28 )
#define COLOR_EYES_RIGHT RGB565( 207, 193, 36 )
#define COLOR_PUPILS RGB565( 0, 0, 0 )

#endif // _config_
