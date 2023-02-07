/*
  Read an 8x8 array of distances from the VL53L5CX
  By: Nathan Seidle
  SparkFun Electronics
  Date: October 26, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to setup and read two sensors. We will hold one
  sensor in reset while we configure the first. You will need to solder
  a wire to each of the sensor's RST pins and connect them to GPIO 14 and 13
  on your plateform.

  Note: The I2C address for the device is stored in NVM so it will have to be set
  at each power on.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/18642
*/

#include <Wire.h>

#include <SparkFun_VL53L5CX_Library.h> //http://librarymanager/All#SparkFun_VL53L5CX

int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output

SparkFun_VL53L5CX myImager1;
int sensorAddress1 = 0x44; //New address of unit without a wire. Valid: 0x08 <= address <= 0x77
int sensorReset1 = 14; //GPIO that is connected to the Reset pin on sensor 1
VL53L5CX_ResultsData measurementData1;

SparkFun_VL53L5CX myImager2;
int sensorAddress2 = 0x29; //Default VL53L5CX - this is the unit we'll hold in reset (has the wire soldered)
int sensorReset2 = 13; //GPIO that is connected to the Reset pin on sensor 2
VL53L5CX_ResultsData measurementData2;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("SparkFun VL53L5CX Imager Example");

  Wire.begin(); //This resets I2C bus to 100kHz
  Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz

  pinMode(sensorReset2, OUTPUT);
  digitalWrite(sensorReset2, HIGH); //Hold sensor 2 in reset while we configure sensor 1

  pinMode(sensorReset1, OUTPUT);
  digitalWrite(sensorReset1, HIGH); //Reset sensor 1
  delay(100);
  digitalWrite(sensorReset1, LOW); //Sensor 1 should now be available at default address 0x29

  Serial.println(F("Initializing sensor 1. This can take up to 10s. Please wait."));
  if (myImager1.begin() == false)
  {
    Serial.println(F("Sensor 1 not found. Check wiring. Freezing..."));
    while (1) ;
  }

  Serial.print(F("Setting sensor 1 address to: 0x"));
  Serial.println(sensorAddress1, HEX);

  if (myImager1.setAddress(sensorAddress1) == false)
  {
    Serial.println(F("Sensor 1 failed to set new address. Please try again. Freezing..."));
    while (1);
  }

  int newAddress = myImager1.getAddress();

  Serial.print(F("New address of sensor 1 is: 0x"));
  Serial.println(newAddress, HEX);

  digitalWrite(sensorReset2, LOW); //Release sensor 2 from reset

  Serial.println(F("Initializing sensor 2. This can take up to 10s. Please wait."));
  if (myImager2.begin() == false)
  {
    Serial.println(F("Sensor 2 not found. Check wiring. Freezing..."));
    while (1) ;
  }

  //Configure both sensors the same just to keep things clean
  myImager1.setResolution(8 * 8); //Enable all 64 pads
  myImager2.setResolution(8 * 8); //Enable all 64 pads

  imageResolution = myImager1.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  myImager1.setRangingFrequency(15);
  myImager2.setRangingFrequency(15);

  myImager1.startRanging();
  myImager2.startRanging();
}

void loop()
{
  //Poll sensor for new data
  if (myImager1.isDataReady() == true)
  {
    if (myImager1.getRangingData(&measurementData1)) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print("\t");
          Serial.print("1:");
          Serial.print(measurementData1.distance_mm[x + y]);
        }
        Serial.println();
      }
      Serial.println();
    }
  }

  if (myImager2.isDataReady() == true)
  {
    if (myImager2.getRangingData(&measurementData2)) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print("\t");
          Serial.print("2:");
          Serial.print(measurementData2.distance_mm[x + y]);
        }
        Serial.println();
      }
      Serial.println();
    }
  }

  delay(5); //Small delay between polling
}