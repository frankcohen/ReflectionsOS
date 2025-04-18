/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Battery.h"

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
  if ( analogVolts < batterylow ) return F("Low");
  if ( analogVolts < battermedium ) return F("Medium");
  if ( analogVolts > batteryhigh ) return F("High");
  return F(" ");
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
  if ( ( millis() - batteryWaitTime ) > ( 60000 * 5 ) )
  {
    batteryWaitTime = millis();

    // int analogValue = analogRead( Battery_Sensor );

    analogVolts = analogReadMilliVolts( Battery_Sensor ) * 5.4014;
    
    String myl = F("Battery voltage = ");
    myl += String( analogVolts );
    myl += F(" ");
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
