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

int Battery::getBatteryLevel()
{
  if ( analogVolts < batterylow ) return 1;
  if ( analogVolts < battermedium ) return 2;
  if ( analogVolts > batteryhigh ) return 3;
  return 0;
}

bool Battery::isBatteryLow()
{
  if ( analogVolts < batterylow ) return true;
  return false;
}

void Battery::loop()
{
  if ( ( millis() - batteryWaitTime ) > ( 60000 * 1 ) )
  {
    batteryWaitTime = millis();

    // int analogValue = analogRead( Battery_Sensor );

    analogVolts = analogReadMilliVolts( Battery_Sensor ) * 5.4014;
    
    String myl = "Battery voltage = ";
    myl += String( analogVolts );
    myl += " ";
    myl += batLevel( analogVolts );

    logger.info( myl );

    y = 140;
    
    myl = "B:";
    myl += getBatteryLevel();
    
    gfx->setFont( &ScienceFair14pt7b );    
    gfx->getTextBounds( myl.c_str(), 0, 0, &x, &y, &w, &h);    

    uint16_t textColor = COLOR_STRIPE_PINK;
    
    gfx->setCursor( (gfx->width() - w) / 2, y );
    gfx->setTextColor( textColor );
    gfx->println( myl );

    /*
    // print out the values you read:
    Serial.print( F( "ADC analog value = " ) );
    Serial.println( analogValue );
    Serial.print( F( "ADC millivolts value = " ) );
    Serial.println( analogVolts );
    */
  }
}
