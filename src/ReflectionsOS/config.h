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

/* Experience videos */

#define OutOfTheBox_video "a1d7900fc81c714ccbdb692231c16d70"

#define Chastise_video "29eebc0d0804260f934575ceaedc50b1"

#define CatsPlayFound_video "8e1cb81b32d7fb3ecb6ac9f16286d1f7"
#define Pounce_video "c4e530e40e5f99e1c8a7afd6cb202423"
#define CatsPlay1_video "52bfa7cd386d6882f49cbdfd04379270"
#define CatsPlay2_video "dfaafa5d7737fa608f562dc65d6a57f4"
#define CatsPlay3_video "e2668735da694ecf0d4208f03e065b1d"
#define CatsPlay4_video "03f9a11699c463e1a5ace6c2f978eecd"
#define CatsPlay5_video "6f37e50a4f148fb74801df10370552bb"
#define CatsPlay6_video "4253ae461e66bd7b04cfd1f1a773f1cd"
#define CatsPlay7_video "7d75dbfe1ac31dd914f6ead0706d38c1"
#define CatsPlay8_video "3f5f5e3f087b6db73a39fad9a7ec4217"

#define EyesFollowFinger_video "f99c9b5031e6102ef45fbdf71cb09134"

#define MysticCat_video "bc481c1a678e0f62e2838e7fc8cd922f"

#define ParallaxCat_video "7f4f62c859d8a8638cb9126c23775372"

#define Shaken_video "538b9f5360a511bf20e1b85a1dc6250d"

#define Sleep_video "c13cccc1f14b431659a70127aa276452"

#define Swipe_video "4e93a6c65511b257e959b0cdc9f9709f"

#define ShowTime_video "0c694842005c6e03d2abe845e530815e"

#define SetTime_video "c0fad0b76200936d73785cfc220d22cd"

#define Getting_Sleepy_video "69e2f5a6b7bfffab4a787be11f57fbcd"

#define videoname_end ".mjpeg"

// WatchFace_Main
#define wfBackground "Face_Main_background_baseline.jpg"
#define wfMainBattery "Battery_"
#define wfMainBattery2 ".png"
#define wfMainHours "Hours_"
#define wfMainHours2 ".png"
#define wfMainMinutes "Minutes_"
#define wfMainMinutes2 ".png"
#define wfMainFace "Main_Cat_Face_"
#define wfMainFace2 ".png"
#define blinkspeed 100
#define wfMainFlip "Flip_"
#define wfMainFlip2 ".png"
#define flipspeed 100

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

// GPS
#define RXPin         18
#define TXPin         17
#define GPSPower      21

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

#define MY_SERVICE_UUID "7d9029fe-48d5-49e0-b9ad-8fd7dac70354"
#define MY_CHARACTERISTIC_UUID "33a03886-0db0-43c8-90a2-bca6a8c7eaf0"

#define bleServerName "REFLECTIONS"

// LED

#define LED_Count 12
#define LED_Type NEOPIXEL
#define LED_Pin 39

// Compass


// TOF Gesture Sensing

#define tof_detectionThreshold 1
#define tof_pollingInterval 1000
#define tof_majorityThreshold 1
#define tof_expiration 2000

// Accellerometer Interrupt pins
#define INT1_PIN GPIO_NUM_14
#define INT2_PIN GPIO_NUM_13

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
#define COLOR_BLACK RGB565(0, 0, 0)
#define COLOR_MAIN_BACK RGB565( 55, 161, 224 )


#endif // _config_
