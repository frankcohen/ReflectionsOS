/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR
*/

#include "Accellerometer.h"

Accellerometer::Accellerometer(){}

void Accellerometer::begin()
{ 
    LIS.begin( Wire ); //IIC init dafault :0x18
    delay(100);
    
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_2G);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_4G);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_8G);
    //  LIS.setFullScaleRange(LIS3DHTR_RANGE_16G);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_1HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_10HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_100HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_200HZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_1_6KHZ);
    //  LIS.setOutputDataRate(LIS3DHTR_DATARATE_5KHZ);

    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
    LIS.setHighSolution(true); //High solution enable
}

boolean Accellerometer::test()
{
  // waits up to 5 seconds to get data from the accellerometer module

  long time = millis();
  while ( millis() < time + 5000 )
  {
     if ( LIS )
     {
        return true;
     }
  }
  return false;    
}

void Accellerometer::printValues()
{
  Serial.print( F( "x:" ) ); Serial.print(LIS.getAccelerationX()); Serial.print( F( "  " ) );
  Serial.print( F( "y:" ) ); Serial.print(LIS.getAccelerationY()); Serial.print( F( "  " ) );
  Serial.print( F( "z:" ) ); Serial.println(LIS.getAccelerationZ());

  //ADC
  Serial.print("adc1:"); Serial.println(LIS.readbitADC1());
  Serial.print("adc2:"); Serial.println(LIS.readbitADC2());
  Serial.print("adc3:"); Serial.println(LIS.readbitADC3());

  //temperature
  Serial.print( F( "temp:" ) );
  Serial.println(LIS.getTemperature()); 
}

void Accellerometer::loop()
{
}
