/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

  Reflections Hoober board as a USB Flash memory stick with contents stored on NAND using ESP32 SD library
  
  What started as a project to wear videos of my children as they grew up on my
  arm as a wristwatch, grew to be a platform for making entertaining experiences.
  
  This software component works with the Hoober board, the second revision to the main board.
  The main board is an ESP32-based platform with OLED display, audio player, flash memory,
  GPS, gesture sensor, and accelerometer/compass
  
  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support
  
  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.
  
  Arduino IDE settings:
  Adafruit Feather ESP32-S3 No PSRAM
  USB mode: USB-OTG
  USB CDC On Boot: Enabled
  USB Firmware MSC On Boot: Disabled
  USB DFU On Boot: Disabled
  
  To get this to work I opened issue: 
  https://github.com/espressif/arduino-esp32/issues/7106
  Since ESP 2.0.7 or greater, it appears this bug is fixed.
  
  ESP32's SD source is at:
  https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
  https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/src/SD.cpp

*/

#include "USBFlashDrive.h"

USBFlashDrive::USBFlashDrive(){}

void USBFlashDrive::begin()
{ 
}

void USBFlashDrive::loop()
{  
}
