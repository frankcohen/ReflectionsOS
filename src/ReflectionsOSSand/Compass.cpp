/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Provides compas/magnetometer readings from the MMC5603NJ device
 
 Datasheet on the sensor:
 - Superior Dynamic Range and Accuracy:
 - ±30 G FSR
 - 20bits operation mode
 - 0.0625mG per LSB resolution
 - 2 mG total RMS noise
 - Enables heading accuracy of 1º
 - Sensor true frequency response up to 1KHz

Pretty cood and tiny chip:
  Full-Scale Range (FSR): ±30 G (GigaGauss), which is ±30,000 mG (since 1 G = 1000 mG).
  Resolution: 20-bit operation mode.
  Resolution per LSB (Least Significant Bit): 0.0625 mG per LSB.

  - ±30 G FSR means the magnetometer is capable of measuring magnetic fields in the range of ±30,000 mG (or ±30 G).

  - 0.0625 mG per LSB is the smallest measurable unit per ADC reading, which corresponds to the resolution of the sensor.

  - 20-bit resolution means the sensor can output a value between 0 and 2^20 (1,048,576 discrete values). Each of these values represents 0.0625 mG of change in the magnetic field.

*/

#include "Compass.h"
#include <Wire.h>

Compass::Compass(){}

void Compass::begin()
{
  mag = Adafruit_MMC5603(12345);
  started = false;
  
  // Initialize the sensor with patched library support for custom chip ID
  if ( !mag.begin( MMC56X3_DEFAULT_ADDRESS, &Wire ) ) 
  {
    if ( !mag.begin( MMC56X3_DEFAULT_ADDRESS, &Wire ) ) 
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

/* Returns compass headings as an int value 0 = North */

int Compass::decodeHeadingVal( float measured_angle ) 
{
  // Adjust angle so that it fits the range [0, 360)
  if (measured_angle < 0) {
    measured_angle += 360;
  }

  // Decoding heading angle with counterclockwise rotation
  if (measured_angle >= 337.25 || measured_angle < 22.5) {
    return 3;  // North
  }
  else if (measured_angle >= 22.5 && measured_angle < 67.5) {
    return 2; // North-East
  }
  else if (measured_angle >= 67.5 && measured_angle < 112.5) {
    return 1;  // East
  }
  else if (measured_angle >= 112.5 && measured_angle < 157.5) {
    return 8; // South-East
  }
  else if (measured_angle >= 157.5 && measured_angle < 202.5) {
    return 7;  // South
  }
  else if (measured_angle >= 202.5 && measured_angle < 247.5) {
    return 6; // South-West
  }
  else if (measured_angle >= 247.5 && measured_angle < 292.5) {
    return 5;  // West
  }
  else if (measured_angle >= 292.5 && measured_angle < 337.25) {
    return 4; // North-West
  }
  else {
    return 3;  // Default case, should not occur.
  }
}

/* Returns compass headings */

String Compass::decodeHeading(float measured_angle) 
{
  // Adjust angle so that it fits the range [0, 360)
  if (measured_angle < 0) {
    measured_angle += 360;
  }

  // Decoding heading angle with counterclockwise rotation
  if (measured_angle >= 337.25 || measured_angle < 22.5) {
    return F("N");  // North
  }
  else if (measured_angle >= 22.5 && measured_angle < 67.5) {
    return F("NE"); // North-East
  }
  else if (measured_angle >= 67.5 && measured_angle < 112.5) {
    return F("E");  // East
  }
  else if (measured_angle >= 112.5 && measured_angle < 157.5) {
    return F("SE"); // South-East
  }
  else if (measured_angle >= 157.5 && measured_angle < 202.5) {
    return F("S");  // South
  }
  else if (measured_angle >= 202.5 && measured_angle < 247.5) {
    return F("SW"); // South-West
  }
  else if (measured_angle >= 247.5 && measured_angle < 292.5) {
    return F("W");  // West
  }
  else if (measured_angle >= 292.5 && measured_angle < 337.25) {
    return F("NW"); // North-West
  }
  else {
    return F("N");  // Default case, should not occur.
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

  // Invert the angle to make counterclockwise decrease the heading
  deg = -deg;
  
  // Normalize angle to be within 0 to 360 degrees
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
  deg += CompassOffsetAdjustment;
  if (deg >= 360) {
    deg -= 360;
  }
  else if (deg < 0) {
    deg += 360;
  }

  // Return the final heading
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
  measured_data[0] = 0.0625 * event.magnetic.x;
  measured_data[1] = 0.0625 * event.magnetic.y;

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

void Compass::callibrate() 
{
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
    Serial.print(F("getHeading "));
    Serial.print(headingValue);
    Serial.print(F(", "));
    Serial.println(decodeHeading(headingValue));
  }
  */
}
