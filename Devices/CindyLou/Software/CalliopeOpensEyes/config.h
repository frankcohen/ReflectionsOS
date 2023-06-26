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

// NAND Storage
#define NAND_SPI_CS   15

// GPS
#define RXPin         18
#define TXPin         17
#define GPSPower      21

// Gesture
#define GesturePower  26

// Audio
#define I2S_bclkPinP       9
#define I2S_wclkPinP      10
#define I2S_doutPinP       8

// I2C
#define I2CSDA        3
#define I2CSCL        4

// BLE

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID_CALLIOPE "7d9029fe-48d5-49e0-b9ad-8fd7dac70354"
#define CHARACTERISTIC_UUID_HEADING "33a03886-0db0-43c8-90a2-bca6a8c7eaf0"
#define bleServerName "CALLIOPE"

// https://cloudcity.starlingwatch.com/listfiles - will show json format of available files
#define cloudCityListFiles "https://cloudcity.starlingwatch.com/listfiles"

// https://cloudcity.starlingwatch.com/ - has recorder
#define cloudCityListRecorder "https://cloudcity.starlingwatch.com/"

// https://cloudcity.starlingwatch.com/files - will show list of files in html view
#define cloudCityFiles "https://cloudcity.starlingwatch.com/files"

// https://cloudcity.starlingwatch.com/<filename.tar> - will serve the tar file
#define cloudCityURL "https://cloudcity.starlingwatch.com/"

// Unimplemented: Please touch using https://cloudcity.starlingwatch.com/touch/243faf385fb4181001c18c5e2f14cb20.tar

#endif // _config_
