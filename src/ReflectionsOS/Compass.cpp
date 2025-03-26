/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Provides compas/magnetometer readings from the MMC56X3 device
 
*/

#include "Compass.h"

Compass::Compass() {}

void Compass::begin()
{
  mag = Adafruit_MMC5603(12345);
  started = false;
  
  // Initialize the sensor with patched library support for custom chip ID
  if ( !mag.begin( MMC56X3_DEFAULT_ADDRESS, &Wire, MMC5603NJ_ID ) ) 
  {
    if ( !mag.begin( MMC56X3_DEFAULT_ADDRESS, &Wire, MMC5603NJ_ID_Alt ) ) 
    {
      Serial.println( F("Compass not detected") );
      return;
    }
  }

  mag.reset();
  delay(1000);

  // Initialize calibration with the first reading
  read_XYZ();
  Max[0] = X; Min[0] = X;
  Max[1] = Y; Min[1] = Y;
  Mid[0] = X;
  Mid[1] = Y;

  ctimer = millis();
  started = true;
}

String Compass::decodeHeading(float measured_angle) 
{
  // Decoding heading angle according to datasheet
  if ( ( measured_angle > 337.25 ) || ( measured_angle < 22.5 ) )
  {
    return "N";
  }
  else {
    if (measured_angle > 292.5) {
      return "NW";
    }
    else if (measured_angle > 247.5) {
      return "W";
    }
    else if (measured_angle > 202.5) {
      return "SW";
    }
    else if (measured_angle > 157.5) {
      return "S";
    }
    else if (measured_angle > 112.5) {
      return "SE";
    }
    else if (measured_angle > 67.5) {
      return "E";
    }
    else {
      return "NE";
    }
  }
}

/*
 * Returns the adjusted heading from the magnetic field reading
 */
float Compass::getHeading(void) 
{
  // Read updated sensor values
  read_XYZ();

  // Calculate the heading using the difference from the midpoint
  float dx = X - Mid[0];
  float dy = Y - Mid[1];

  // Use atan2 for robust handling of quadrants and zero divisions
  float deg = atan2(dy, dx) * (180.0 / PI);
  if (deg < 0) {
    deg += 360;
  }

  // Apply magnetic declination correction
  deg += DECLINATION;
  if (deg >= 360) {
    deg -= 360;
  }
  else if (deg < 0) {
    deg += 360;
  }

  // Apply board fixed offset correction (adjust for sensor mounting)
  deg += (180 + 40);
  if (deg >= 360) {
    deg -= 360;
  }

  Serial.print("Compass: ");
  Serial.println(deg);
  
  return deg;
}

/*
 * Read X, Y components of the magnetic field and update calibration
 */
void Compass::read_XYZ()
{
  sensors_event_t event;
  mag.getEvent(&event);

  // Convert raw data to mG using the conversion factor from the datasheet
  float measured_data[2];
  measured_data[0] = 0.48828125 * event.magnetic.x;
  measured_data[1] = 0.48828125 * event.magnetic.y;

  X = measured_data[0];
  Y = measured_data[1];

  // Update calibration values (max, min, and midpoints)
  if (measured_data[0] > Max[0]) { 
    Max[0] = measured_data[0];
  }
  if (measured_data[0] < Min[0]) { 
    Min[0] = measured_data[0];
  }
  if (measured_data[1] > Max[1]) { 
    Max[1] = measured_data[1];
  }
  if (measured_data[1] < Min[1]) { 
    Min[1] = measured_data[1];
  }
  Mid[0] = (Max[0] + Min[0]) / 2;
  Mid[1] = (Max[1] + Min[1]) / 2;
}

String Compass::updateHeading() {
  // This stub returns a simple string formatted heading.
  float headingValue = getHeading();
  return String(headingValue);
}

void Compass::callibrate() {
  // Implement a proper calibration routine here, for example:
  // instruct the user to rotate the sensor through all directions
  // and possibly record multiple values to refine Max, Min, and Mid.
}

boolean Compass::test()
{
  return started;
}

void Compass::loop()
{
  /*
  if ( millis() - ctimer > 2000 )
  {
    ctimer = millis();
    float headingValue = getHeading();
    Serial.print("getHeading ");
    Serial.print(headingValue);
    Serial.print(", ");
    Serial.println(decodeHeading(headingValue));
  }
  */
}
