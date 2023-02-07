/*
Reflections, distributed entertainment device

Accellerometer test, LIS331 chip

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for story-level code, setup, and loops. Additional source:
Utils ( General purpose tasks )
Audio
Video
Storage ( Gets media/data from server, over Wifi )

Depends on:
JPEGDEC:     https://github.com/bitbank2/JPEGDEC.git
ArduinoJSON: https://arduinojson.org/
ESP32-targz: https://github.com/tobozo/ESP32-targz
Arduino_GFX: https://github.com/moononournation/Arduino_GFX
esp32FOTA:   https://github.com/chrisjoyce911/esp32FOTA
Adafruit LIS331: https://github.com/adafruit/Adafruit_LIS331

// Basic demo for accelerometer readings from Adafruit H3LIS331

*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_H3LIS331.h>
#include <Adafruit_Sensor.h>

// Used for software SPI
#define H3LIS331_SCK 13
#define H3LIS331_MISO 12
#define H3LIS331_MOSI 11
// Used for hardware & software SPI
#define H3LIS331_CS 10


Adafruit_H3LIS331 lis = Adafruit_H3LIS331();

void setup(void) {
  Serial.begin(115200);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.setDebugOutput(true);
  Serial.println("Reflections Acclerometer research");

  Serial.println("H3LIS331 test");

  Wire.begin(3,4);

//  if (!lis.begin_SPI(H3LIS331_CS)) {
//  if (!lis.begin_SPI(H3LIS331_CS, H3LIS331_SCK, H3LIS331_MISO, H3LIS331_MOSI)) {
 if (! lis.begin_I2C( )) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("H3LIS331 found!");

  // lis.setRange(H3LIS331_RANGE_100_G);   // 100, 200, or 400 G!
  Serial.print("Range set to: ");
  switch (lis.getRange()) {
    case H3LIS331_RANGE_100_G: Serial.println("100 g"); break;
    case H3LIS331_RANGE_200_G: Serial.println("200 g"); break;
    case H3LIS331_RANGE_400_G: Serial.println("400 g"); break;
  }
  // lis.setDataRate(LIS331_DATARATE_1000_HZ);
  Serial.print("Data rate set to: ");
  switch (lis.getDataRate()) {

    case LIS331_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS331_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS331_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS331_DATARATE_400_HZ: Serial.println("400 Hz"); break;
    case LIS331_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: Serial.println("0.5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: Serial.println("1 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: Serial.println("2 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: Serial.println("5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: Serial.println("10 Hz Low Power"); break;

  }
}

void loop() {
  /* Get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
  Serial.println(" m/s^2 ");

  /* Alternately, given the range of the H3LIS331, display the results measured in g */
  // Serial.print("\t\tX:"); Serial.print(event.acceleration.x / SENSORS_GRAVITY_STANDARD);
  // Serial.print(" \tY: "); Serial.print(event.acceleration.y / SENSORS_GRAVITY_STANDARD);
  // Serial.print(" \tZ: "); Serial.print(event.acceleration.z / SENSORS_GRAVITY_STANDARD);
  // Serial.println(" g");

  delay(1000);
}
