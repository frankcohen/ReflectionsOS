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
#include "SPI.h"
#include "SD.h"
#include "FS.h"

Utils::Utils(){}

void Utils::begin(){}

void Utils::smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

void Utils::SetupHardware()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(8000000);

  Serial.begin(115200);
  smartdelay(1000);
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
