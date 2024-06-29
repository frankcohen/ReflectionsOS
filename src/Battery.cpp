/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Battery.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

Battery::Battery(){}

void Battery::begin()
{ 
  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);
  batteryWaitTime = millis();
}

bool Battery::test()
{
  return true;
}

// 3500 = battery at empty

String Battery::batLevel( float analogVolts )
{
  if ( analogVolts < batterylow ) return "Low";
  if ( analogVolts < battermedium ) return "Medium";
  if ( analogVolts > batteryhigh ) return "High";
  return " ";
}

bool Battery::isBatteryLow()
{
  if ( analogVolts < batterylow ) return true;
  return false;
}

void Battery::loop()
{
  if ( ( millis() - batteryWaitTime ) > 60000 )
  {
    batteryWaitTime = millis();

    // int analogValue = analogRead( Battery_Sensor );

    analogVolts = analogReadMilliVolts( Battery_Sensor ) * 5.4014;
    
    String myl = "Battery voltage = ";
    myl += String( analogVolts );
    myl += " ";
    myl += batLevel( analogVolts );

    logger.info( myl );

    /*
    // print out the values you read:
    Serial.print( F( "ADC analog value = " ) );
    Serial.println( analogValue );
    Serial.print( F( "ADC millivolts value = " ) );
    Serial.println( analogVolts );
    */
  }
}

/* Draws sprite, does not draw for transparent pixels */

void Battery::drawSpriteOverBackground(const uint16_t *sprite, int16_t spriteWidth, int16_t spriteHeight, int16_t x, int16_t y, uint16_t transparent) {
  for (int16_t i = 0; i < spriteHeight; i++) {
    for (int16_t j = 0; j < spriteWidth; j++) {
      uint16_t pixelColor = sprite[i * spriteWidth + j];
      if (pixelColor != transparent) {
        // gfx->drawPixel(x + j, y + i, pixelColor);
      }
    }
  }
}

void Battery::drawLowBatteryIcon()
{
  // Draws low battery warning icon

  // drawSpriteOverBackground( BatteryIcon, BATTERYLOW_WIDTH, BATTERYLOW_HEIGHT, BATTERY_X, BATTERY_Y, BATTERY_TRANSPARENT_COLOR );
}


