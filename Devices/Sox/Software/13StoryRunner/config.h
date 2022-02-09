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
#define wifiPass ""

#define MJPEG_FILENAME ".mjpeg"
#define TAR_FILENAME ".tar"
#define JSON_FILENAME ".json"

#define BUTTON_LEFT   36
#define BUTTON_RIGHT  39

// Digital I/O used
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

#define SD_CS         4
#define SPI_DisplayDC 16
#define SPI_DisplayCS 32
#define SPI_DisplayRST 17

#define I2S_DOUT      27
#define I2S_BCLK      26
#define I2S_LRC       25

#endif // _config_
