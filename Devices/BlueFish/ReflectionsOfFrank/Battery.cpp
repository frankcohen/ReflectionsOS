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

void Battery::loop()
{
  if ( ( millis() - batteryWaitTime ) > 30000 )
  {
    batteryWaitTime = millis();

    // int analogValue = analogRead( Battery_Sensor );

    float analogVolts = analogReadMilliVolts( Battery_Sensor ) * 5.4014;
    
    String myl = "Battery voltage = ";
    myl += String( analogVolts );
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
