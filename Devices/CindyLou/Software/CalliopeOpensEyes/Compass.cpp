/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on Adafruit's MMC5603 library https://github.com/adafruit/Adafruit_LIS2MDL


 Thanks for Digilent for example code to decode compass headings:
 https://digilent.com/reference/pmod/pmodcmps2/start

 Thanks to jremington and the Arduino forums for the magnetometer adjustment code
 https://forum.arduino.cc/t/magnetometer-calibration/906862/5
 based on the best tutorial on magnetometer calibration I've read
 https://thecavepearlproject.org/2015/05/22/calibrating-any-compass-or-accelerometer-for-arduino/

 The earth's magnetic field varies according to its location.
 Add or subtract a constant to get the right value
 of the magnetic field using the following site
 http://www.ngdc.noaa.gov/geomag-web/#declination
*/

#define DECLINATION -0.08387 // declination (in degrees) in Silicon Valley, California USA

// Calibration of Horton Red
//Mag Minimums: -34.98  -60.03  -110.38
//Mag Maximums: 23.66  0.00  0.00

// Horton Black
//Mag Minimums: 0.00  -16.62  -50.97
//Mag Maximums: 86.56  38.40  0.00

#include "Compass.h"

Compass::Compass(){}

void Compass::begin()
{ 
  mag = Adafruit_MMC5603(12345);
  started = false;
  
  /* Initialise the sensor */
  if ( !mag.begin( MMC56X3_DEFAULT_ADDRESS, &Wire ) ) 
  {
    Serial.println( F( "Compass not detected" ) );
  }

  mag.reset();
  delay(1000);
  mag.magnetSetReset();
  delay(1000);

  MagMinX = MagMaxX = MagMinY = MagMaxY = MagMinZ = MagMaxZ = 0;
  lastDisplayTime = millis();

  started = true;
}

void Compass::setHeading( String _heading )
{
  heading = _heading;
}

String Compass::getHeading()
{
  return heading;
}

String Compass::updateHeading()
{
  sensors_event_t event;
  mag.getEvent(&event);

  float Pi = 3.14159;

  // Calculate the angle of the vector y,x
  float measured_angle = ( atan2( event.magnetic.y, event.magnetic.x ) - DECLINATION ) * 180 / Pi;

  // Normalize to 0-360
  if (measured_angle < 0)
  {
    measured_angle = 360 + measured_angle;
  }

  if ( ( measured_angle >= 265 ) && ( measured_angle <= 278 ) )
    {
      return F( "N" );
    }
    else if ( ( measured_angle >= 294 ) && ( measured_angle <= 296 ) )
    {
      return F( "NE" );
    }
    else if ( ( measured_angle >= 286 ) && ( measured_angle <= 293 ) )
    {
      return F( "E" ) ;
    }
    else if ( ( measured_angle >= 269 ) && ( measured_angle <= 285 ) )
    {
      return F( "SE" );
    }
    else if ( ( measured_angle >= 256 ) && ( measured_angle <= 264 ) )
    {
      return F( "S" );
    }
    else if ( ( measured_angle >= 236 ) && ( measured_angle <= 255 ) )
    {
      return F( "SW" );
    }
    else if ( ( measured_angle >= 229 ) && ( measured_angle <= 249 ) )
    {
      return F( "W" );
    }
    else if ( ( measured_angle >= 234 ) && ( measured_angle <= 254 ) )
    {
      return ( F( "NW" ) );
    }

    else if ( ( measured_angle >= 17 ) && ( measured_angle <= 37 ) )
    {
      return F( "N" );
    }
    else if ( ( measured_angle >= 5 ) && ( measured_angle <= 16 ) )
    {
      return F( "NE" );
    }
    else if ( ( measured_angle >= 0 ) && ( measured_angle <= 4 ) )
    {
      return F( "E" );
    }
    else if ( ( measured_angle >= 340 ) && ( measured_angle <= 352 ) )
    {
      return F( "SE" );
    }
    else if ( ( measured_angle >= 352 ) && ( measured_angle <= 359 ) )
    {
      return F( "S" );
    }
    else if ( ( measured_angle >= 0 ) && ( measured_angle <= 12 ) )
    {
      return F( "SW" );
    }
    else if ( ( measured_angle >= 12 ) && ( measured_angle <= 30 ) )
    {
      return F( "W" );
    }
    else if ( ( measured_angle >= 18 ) && ( measured_angle <= 38 ) )
    {
      return "NW";
    }

    else {
      Serial.print( F( "U " ) );
      Serial.println( measured_angle );
      return "U ";
    }
}

boolean Compass::test()
{
  return started;
}

void Compass::callibrate()
{
  sensors_event_t magEvent;
  mag.getEvent(&magEvent);

  if ( ( magEvent.magnetic.x < -360 ) || ( magEvent.magnetic.y < -360 ) || ( magEvent.magnetic.z < -350 ) )
  {
    return;
  }

  if (magEvent.magnetic.x < MagMinX) MagMinX = magEvent.magnetic.x;
  if (magEvent.magnetic.x > MagMaxX) MagMaxX = magEvent.magnetic.x;

  if (magEvent.magnetic.y < MagMinY) MagMinY = magEvent.magnetic.y;
  if (magEvent.magnetic.y > MagMaxY) MagMaxY = magEvent.magnetic.y;

  if (magEvent.magnetic.z < MagMinZ) MagMinZ = magEvent.magnetic.z;
  if (magEvent.magnetic.z > MagMaxZ) MagMaxZ = magEvent.magnetic.z;

  if ((millis() - lastDisplayTime) > 1000)  // display once/second
  {
    Serial.print( F( "Mag Minimums: " )); Serial.print(MagMinX); Serial.print(F( "  " ));Serial.print(MagMinY); Serial.print( F( "  " ) ); Serial.print(MagMinZ); Serial.println();
    Serial.print("Mag Maximums: "); Serial.print(MagMaxX); Serial.print("  ");Serial.print(MagMaxY); Serial.print("  "); Serial.print(MagMaxZ); Serial.println(); Serial.println();
    lastDisplayTime = millis();
  }
}

void Compass::loop()
{
}
