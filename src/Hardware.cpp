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

Hardware::Hardware(){}

void Hardware::begin()
{ 
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);

  Wire.begin(I2CSDA, I2CSCL);  // Initialize I2C bus

  // Starts SD/NAND storage component

  pinMode(NAND_SPI_CS, OUTPUT );
  digitalWrite(NAND_SPI_CS, HIGH);
  delay(1000);
  digitalWrite(NAND_SPI_CS, LOW);

  // Configure display pins

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, LOW);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);

  // Turns the speaker amp on
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, HIGH);

  if ( ! SD.begin( NAND_SPI_CS, SPI, 10000000) )
  {
    Serial.println(F("SD storage failed"));
    Serial.println(F("Stopping"));
    NANDMounted = false;
    while(1)  // Stopping
    {
      delay(1000);
    };
  }
  else
  {
    Serial.println(F("SD storage mounted"));
    NANDMounted = true;
  }
}

bool Hardware::getMounted()
{
  return NANDMounted;
}

void Hardware::loop()
{
}
