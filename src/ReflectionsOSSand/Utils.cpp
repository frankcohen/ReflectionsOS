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

/*
* Pretty print file contents in Hex format to serial monitor
*/

void Utils::hexDump( File file )
{
  const byte bytesPerRow = 16; // 4 as minimum

  char hexVal[9];
  byte buffer[bytesPerRow];

  Serial.print(F("File: "));
  Serial.println(file.name());
  Serial.println();

  // Writting header
  Serial.print(F("Offset    "));
  Serial.print(F("Hexadecimal ")); //added )

  // Pad with spaces depending on the amount of bytes per row (it should align with the text below)
  for (unsigned int spaces = (bytesPerRow - 4) * 3; spaces; spaces--)
  {
    Serial.write(' '); //put inside a {}
  }

  Serial.println(F(" ASCII"));

  file.seek(0);
  // Typing file's content
  while ( file.available() )
  {
    // Print offset
    sprintf(hexVal, "%08lX", file.position());

    Serial.print(hexVal);
    Serial.print(F("  ")); // to leave two spaces between offset and first hex character
    byte amount = file.read( buffer, bytesPerRow );

    if ( amount > 0 )
    {
      // Print hex values
      for (int i = 0; i < amount; i++)
      {
        sprintf(hexVal, "%02X ", buffer[i]);
        Serial.print(hexVal);
      }

      // Fill with spaces in case we couldn't fill an entire row (due to reaching the end of the file)
      for (unsigned int spaces = (bytesPerRow - amount) * 3; spaces; spaces--)
        Serial.write(' ');

      Serial.write(' '); // Another extra space I've missed before
      // Print ASCII values
      for (byte i = 0; i < amount; i++)
      {
        // Printable characters appear as they are, non-printable appear as a dot/period (.)
        Serial.write(buffer[i] > 31 && buffer[i] != 127 ? buffer[i] : '.');
      }

    }

    Serial.println();
  }

  file.close();
  Serial.println(F("                  **File closed**"));
}

void Utils::WireScan() 
{
  byte error, address;
  int nDevices;

  Serial.println( F( "Scanning I2C bus" ) );

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    /*
    Yertel and later boards
    I2C device found at address 0x18 (24)  LIS331_DEFAULT_ADDRESS - accelerometer
    I2C device found at address 0x29 (41)  Gesture sensor
    I2C device found at address 0x30 (48)  Magnetometer, compass
    I2C device found at address 0x5A (90)  Haptic controller
    */

    if (error == 0)
    {
      Serial.print( F( "0x" ) );
      if (address<16) Serial.print(F("0"));
      
      Serial.print(address,HEX);
      bool lastv = false;
      if ( address == 24 )
      {
        Serial.print( F(" 24 accelerometer") );
        lastv = true;
      }
      if ( address == 41 )
      {
        Serial.print( F(" 41 TOF gesture") );
        lastv = true;
      }
      if ( address == 48 )
      {
        Serial.print( F(" 48 compass") );
        lastv = true;
      }
      if ( address == 90 )
      {
        Serial.print( F(" 90 haptic") );
        lastv = true;
      }

      if ( lastv )
      {
        Serial.println( F("") );
      }

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
}

void Utils::begin()
{ 
}

void Utils::loop()
{
}
