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

#define cloudCityURL "35.163.96.119"
#define cloudCityPort 8088

#define wifiSSID "FranksFreeInternet"
//#define wifiSSID "Franks_Free_Internet_Guest"
#define wifiPass ""

#define MJPEG_FILENAME ".mjpeg"
#define TAR_FILENAME ".tar"
#define JSON_FILENAME ".json"

// Digital I/O used
#define SPI_MOSI      35
#define SPI_MISO      37
#define SPI_SCK       36

#define SD_CS         10

#define SPI_DisplayDC 6
#define SPI_DisplayCS 5
#define SPI_DisplayRST 9

#define I2S_DOUT      27
#define I2S_BCLK      26
#define I2S_LRC       25

#endif // _config_
