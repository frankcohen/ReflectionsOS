# Libraries for Reflections

fcohen@starlingwatch.com
September 28, 2023

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with display,
audio player, flash memory, GPS, gesture sensor, accelerometer/compass, and more.

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

## Depends on these libraries

For convenience, these libraries are in the [https://github.com/frankcohen/ReflectionsOS/libraries](https://github.com/frankcohen/ReflectionsOS/libraries) directory. Copy the contents to your Arduino IDE installation under documents/libraries.

```
esp32FOTA, OTA updates, https://github.com/chrisjoyce911/esp32FOTA
Adafruit DRV2605 Library, haptic controller, https://github.com/adafruit/Adafruit_DRV2605_Library
Adafruit MMC56x3, compass, magnetometer, https://github.com/adafruit/Adafruit_MMC56x3
Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
Adafruit seesaw Library, https://github.com/adafruit/Adafruit_Seesaw
ArduinoJson, https://arduinojson.org/
ESP32-targz, https://github.com/tobozo/ESP32-targz/
ESP32_HTTPS_Server, https://github.com/fhessel/esp32_https_server
ESP8266Audio, https://github.com/earlephilhower/ESP8266Audio
FastLED, https://github.com/FastLED/FastLED
GFX Library for Arduino, https://github.com/moononournation/Arduino_GFX
JPEGDEC, https://github.com/bitbank2/JPEGDEC.git
NimBLE-Arduino, https://github.com/h2zero/NimBLE-Arduino
LISDHTR Acellerometer, https://travis-ci.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR.svg?branch=master
SparkFun_VL53L5CX_Arduino_Library, https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
Time, https://playground.arduino.cc/Code/Time/
TinyGPSPlus-ESP32, https://github.com/Tinyu-Zhao/TinyGPSPlus-ESP32
URLEncode, https://github.com/plageoj/urlencode
WiFiManager by Tzapu, https://github.com/tzapu/WiFiManager
```

Depends on Esspresif ESP32 libraries at
[https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)

```
Using Arduino IDE 2.x, use Tools menu options:
Board: ESP32S3 Dev Module
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
Partition Scheme: Defaults 4MB with Spiffs (1.2 MB APP/1.5MB SPIFFS)
PSRAM: Disabled
Upload Mode: UART0/Hardware CDC
Upload Speed 921600
USB Mode: Hardware CDC and JTAG
```
