/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Hardware.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;

Hardware::Hardware(){}

void Hardware::begin()
{
  // Starts SD/NAND storage component

  pinMode( NAND_SPI_PWR, OUTPUT);
  digitalWrite( NAND_SPI_PWR, HIGH);
  delay(1000);
  digitalWrite( NAND_SPI_PWR, LOW);

  pinMode(NAND_SPI_CS, OUTPUT);
  digitalWrite(NAND_SPI_CS, HIGH);

  powerUpComponents(); 

  // Create an SPIClass instance
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, NAND_SPI_CS);

  if ( ! SD.begin( NAND_SPI_CS, SPI, SPI_SPEED) )
  {
    Serial.println(F("SD storage failed"));
    Serial.println(F("Stopping"));
    NANDMounted = false;
    video.stopOnError( F( "Storage" ), F( "failed" ), F( "to" ), F( "start" ), F( " " ) );
  }
  else
  {
    Serial.println(F("SD storage mounted"));
    NANDMounted = true;
  }

  // Begin SPI communication with desired settings
  //SPISettings spiSettings(SPI_SPEED, MSBFIRST, SPI_MODE0);

  Wire.begin(I2CSDA, I2CSCL);  // Initialize I2C bus
  Wire.setClock(400000);
}

void Hardware::powerUpComponents()
{ 
  // Configure display pins

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);      // Turns backlight on = low, off = high
  digitalWrite(Display_SPI_BK, HIGH);

  pinMode( ACCEL_INT1_PIN, INPUT );     // Accelerometer awake on movement pins
  pinMode( ACCEL_INT2_PIN, INPUT );

  pinMode(LED_Pin, OUTPUT);

  pinMode( TOFPower, OUTPUT);    // Power control for TOF sensor
  digitalWrite( TOFPower, LOW);

  // Turns the speaker amp on
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);    // HIGH is on

  // Turn GPS module on
  pinMode(GPSPower, OUTPUT);
  digitalWrite(GPSPower, HIGH);      // HIGH is on

  uint32_t seed = esp_random();
  randomSeed( seed );
}

void Hardware::powerDownComponents()
{
  pinMode(Display_SPI_BK, OUTPUT);  // Turns backlight on = low, off = high
  digitalWrite( Display_SPI_BK, HIGH );

  pinMode( TOFPower, OUTPUT);       // Power control for TOF sensor
  digitalWrite( TOFPower, HIGH);     // Low is on

  // Turns the speaker amp on
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);    // HIGH is on

  // Turn GPS module on
  pinMode(GPSPower, OUTPUT);
  digitalWrite(GPSPower, LOW);     // HIGH is on
}

bool Hardware::getMounted()
{
  return NANDMounted;
}

void Hardware::loop()
{
}
