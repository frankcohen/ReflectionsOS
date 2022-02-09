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

This file is for general purpose tasks

*/

#include <ArduinoJson.h>
#include "Utils.h"
#include "config.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

Utils::Utils(){}

void Utils::begin()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  pinMode(SPI_DisplayCS, OUTPUT);
  digitalWrite(SPI_DisplayCS, HIGH);

  pinMode(SPI_DisplayDC, OUTPUT);
  digitalWrite(SPI_DisplayDC, HIGH);

  pinMode(SPI_DisplayRST, OUTPUT);
  digitalWrite(SPI_DisplayRST, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(4000000);  // at 8000000 I get CPU panic reboots

  //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  Serial.begin(115200);
  smartdelay(1000);
  Serial.println("Reflections Story Runner");
  Serial.println("HTTP-based MJPEG Movie and Audio Player");

  if ( ! SD.begin(SD_CS) )
  {
    Serial.println(F("SD card failed"));
    while(1);
  }
  else
  {
    Serial.println(F("SD card mounted"));
  }
}

void Utils::smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // feel free to do something here
  } while (millis() - start < ms);
}
