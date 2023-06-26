/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
*/

#include "Utils.h"

#define WIRE Wire

Utils::Utils(){}

void Utils::WireScan() 
{
  byte error, address;
  int nDevices;

  Serial.println( F( "Scanning..." ) );

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    if (error == 0)
    {
      Serial.print( F( "I2C device found at address 0x" ) );
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print( F( "Unknown error at address 0x" ) );
      if (address<16) 
        Serial.print( F( "0" ) );
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println(F( "No I2C devices found\n" ) );
  else
    Serial.println( F( "done\n" ) );

  delay(5000);           // wait 5 seconds for next scan
}

void Utils::begin()
{ 
}

void Utils::loop()
{
}
