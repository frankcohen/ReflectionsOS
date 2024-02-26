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
#define SPI_SPEED     40000000

// NAND Storage
#define NAND_SPI_CS   15
#define NAND_BASE_DIR "REFLECTIONS"
#define LOGNAME_START "/REFLECTIONS/log"

// Accelerometer template storage
#define ACCEL_BASE_DIR "agesture"
#define ACCEL_BASE_FILE "cancel"
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

#define SERVICE_UUID_CALLIOPE "7d9029fe-48d5-49e0-b9ad-8fd7dac70354"
#define CHARACTERISTIC_UUID_HEADING "33a03886-0db0-43c8-90a2-bca6a8c7eaf0"
#define bleServerName "REFLECTIONS"

// LED

#define LED_Count 12
#define LED_Type NEOPIXEL
#define LED_Pin 39

// Compass

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

#endif // _config_
