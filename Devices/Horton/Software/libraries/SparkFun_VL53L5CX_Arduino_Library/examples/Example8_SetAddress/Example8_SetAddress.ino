/*
  Read an 8x8 array of distances from the VL53L5CX
  By: Nathan Seidle
  SparkFun Electronics
  Date: October 26, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to set a custom address for the VL53L5CX.

  Note: The I2C address for the device is stored in NVM so it will have to be set
  at each power on.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/18642
*/

#include <Wire.h>

#include <SparkFun_VL53L5CX_Library.h> //http://librarymanager/All#SparkFun_VL53L5CX

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("SparkFun VL53L5CX Imager Example");

  Wire.begin(); //This resets I2C bus to 100kHz
  Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz

  Serial.println(F("Initializing sensor board. This can take up to 10s. Please wait."));
  if (myImager.begin() == false)
  {
    Serial.println(F("Sensor not found - running scanner"));
    i2cScanner();
  }
  else
  {
    while (Serial.available()) //Clear incoming key presses
      Serial.read();

    Serial.println(F("Press any key to set device address to 0x44"));
    int deviceAddress = 0x44; //Valid: 0x08 <= address <= 0x77

    while (Serial.available() == false) //Clear incoming key presses
      ; //Do nothing

    if (myImager.setAddress(deviceAddress) == false)
    {
      Serial.println(F("Device failed to set new address. Please try again. Freezing..."));
      while (1);
    }

    delay(500);

    int newAddress = myImager.getAddress();

    Serial.print(F("New address is: 0x"));
    Serial.println(newAddress, HEX);

    Serial.println(F("See advanced I2C example to start sensor with new address."));

    myImager.setResolution(8 * 8); //Enable all 64 pads

    imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
    imageWidth = sqrt(imageResolution); //Calculate printing width

    myImager.startRanging();
  }
}

void loop()
{
  //Poll sensor for new data
  if (myImager.isDataReady() == true)
  {
    if (myImager.getRangingData(&measurementData)) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
      {
        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print("\t");
          Serial.print(measurementData.distance_mm[x + y]);
        }
        Serial.println();
      }
      Serial.println();
    }
  }

  delay(5); //Small delay between polling
}

void i2cScanner()
{
  while (1)
  {
    Serial.println(F("Scanning for I2C devices"));
    for (byte address = 1; address < 127; address++ )
    {
      Wire.beginTransmission(address);
      if (Wire.endTransmission() == 0)
      {
        Serial.print("Device found at address 0x");
        if (address < 0x10)
          Serial.print("0");
        Serial.println(address, HEX);
      }
    }
    Serial.println("Done");
    delay(250);
  }
}
