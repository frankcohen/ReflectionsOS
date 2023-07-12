/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Horton and CindyLou boards use MMC5603NJ magnetometer and LIS3DHTR_C15134 accellerometer

 Depends on Adafruit_MMC56x3 library https://github.com/adafruit/Adafruit_MMC56x3

 Thanks for Digilent for example code to decode compass headings:
 https://digilent.com/reference/pmod/pmodcmps2/start
 https://digilent.com/reference/pmod/pmodcmps2/start#example_projects

 Thanks to jremington and the Arduino forums for the magnetometer adjustment code
 https://forum.arduino.cc/t/magnetometer-calibration/906862/5
 based on the best tutorial on magnetometer calibration I've read
 https://thecavepearlproject.org/2015/05/22/calibrating-any-compass-or-accelerometer-for-arduino/

*/

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

  //initialize minimum, mid and maximum values
  for (int i = 0; i < 2; i++) {
    Max[i] = -32768;  //smallest int on 16 bits
    Min[i] = 32767;  //largest int on 16 bits
    Mid[i] = 0;
  }

  lastDisplayTime = millis();

  started = true;
}

String Compass::decodeHeading(float measured_angle) 
{
  //decoding heading angle according to datasheet
  if (measured_angle > 337.25 | measured_angle < 22.5) {
    //Serial.println("North");
    return "N";
  }
  else {
    if (measured_angle > 292.5) {
      //Serial.println("North-West");
      return "NW";
    }
    else if (measured_angle > 247.5) {
      //Serial.println("West");
      return "W";
    }
    else if (measured_angle > 202.5) {
      //Serial.println("South-West");
      return "SW";
    }
    else if (measured_angle > 157.5) {
      //Serial.println("South");
      return "S";
    }
    else if (measured_angle > 112.5) {
      //Serial.println("South-East");
      return "SE";
    }
    else if (measured_angle > 67.5) {
      //Serial.println("East");
      return "E";
    }
    else {
      //Serial.println("North-East");
      return "NE";
    }
  }
}

/*
 * Returns the adjusted heading from the magnetic field reading
 */
 
float Compass::getHeading(void) {

  float components[2];

  read_XYZ();  //read X, Y, Z components of the magnetic field

  components[0] = X;  //save current results
  components[1] = Y;

  //mag.reset();
  //delay(100);
  //read_XYZ();  //read X, Y, Z components of the magnetic field

  //eliminate offset from all components
  //components[0] = (components[0] - X) / 2.0;
  //components[1] = (components[1] - Y) / 2.0;

  //variables for storing partial results
  float temp0 = 0;
  float temp1 = 0;
  //and for storing the final result
  float deg = 0;

  //calculate heading from components of the magnetic field
  //the formula is different in each quadrant
  if (components[0] < Mid[0])
  {
    if (components[1] > Mid[1])
    {
      //Quadrant 1
      temp0 = components[1] - Mid[1];
      temp1 = Mid[0] - components[0];
      deg = 90 - atan(temp0 / temp1) * (180 / 3.14159);
    }
    else
    {
      //Quadrant 2
      temp0 = Mid[1] - components[1];
      temp1 = Mid[0] - components[0];
      deg = 90 + atan(temp0 / temp1) * (180 / 3.14159);
    }
  }
  else {
    if (components[1] < Mid[1])
    {
      //Quadrant 3
      temp0 = Mid[1] - components[1];
      temp1 = components[0] - Mid[0];
      deg = 270 - atan(temp0 / temp1) * (180 / 3.14159);
    }
    else
    {
      //Quadrant 4
      temp0 = components[1] - Mid[1];
      temp1 = components[0] - Mid[0];
      deg = 270 + atan(temp0 / temp1) * (180 / 3.14159);
    }
  }

  //correct heading
  deg += DECLINATION;
  if (DECLINATION > 0)
  {
    if (deg > 360) {
      deg -= 360;
    }
  }
  else
  {
    if (deg < 0) {
      deg += 360;
    }
  }

  return deg;
}

/*
 * Read X, Y components of the magnetic field
*/

void Compass::read_XYZ()
{
  sensors_event_t event;
  mag.getEvent(&event);

  //initialize array for data
  float measured_data[2];

  //reconstruct raw data
  measured_data[0] = event.magnetic.x;  //x
  measured_data[1] = event.magnetic.y;  //y

  //convert raw data to mG per https://digilent.com/blog/how-to-convert-magnetometer-data-into-compass-heading/
  for (int i = 0; i < 2; i++) {
    measured_data[i] = 0.48828125 * (float)measured_data[i];
  }

  X = measured_data[0];
  Y = measured_data[1];

  //correct minimum, mid and maximum values
  if (measured_data[0] > Max[0]) { //x max
    Max[0] = measured_data[0];
  }
  if (measured_data[0] < Min[0]) { //x min
    Min[0] = measured_data[0];
  }
  if (measured_data[1] > Max[1]) { //y max
    Max[1] = measured_data[1];
  }
  if (measured_data[1] < Min[1]) { //y min
    Min[1] = measured_data[1];
  }
  for (int i = 0; i < 2; i++) { //mid
    Mid[i] = (Max[i] + Min[i]) / 2;
  }
}

boolean Compass::test()
{
  return started;
}

void Compass::loop()
{
}
