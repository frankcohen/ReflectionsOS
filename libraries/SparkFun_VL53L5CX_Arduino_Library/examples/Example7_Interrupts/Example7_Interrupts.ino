/*
  Read an 8x8 array of distances from the VL53L5CX
  By: Nathan Seidle
  SparkFun Electronics
  Date: October 26, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to read the interrupt from the VL53L5CX.

  The INT pin is active low, output, by default. For this example you'll need to solder
  a wire from the INT pin to pin 4 (or any interrupt capable pin) on your microcontroller

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/18642

*/

#include <Wire.h>

#include <SparkFun_VL53L5CX_Library.h> //http://librarymanager/All#SparkFun_VL53L5CX

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output

#define INT_PIN 4 //Connect VL53L5CX INT pin to pin 4 on your microcontroller
volatile bool dataReady = false; //Goes true when interrupt fires

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("SparkFun VL53L5CX Imager Example");

  Wire.begin(); //This resets I2C bus to 100kHz
  Wire.setClock(1000000); //Sensor has max I2C freq of 1MHz

  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");
  if (myImager.begin() == false)
  {
    Serial.println(F("Sensor not found - check your wiring. Freezing"));
    while (1) ;
  }

  myImager.setResolution(8 * 8); //Enable all 64 pads

  imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  myImager.setRangingFrequency(15);

  // Attach the interrupt
  attachInterrupt(digitalPinToInterrupt(INT_PIN), interruptRoutine, FALLING);
  Serial.println(F("Interrupt pin configured."));

  myImager.startRanging();
}

void loop()
{
  //ISR will let us know when new data is ready
  if (dataReady == true)
  {
    dataReady = false;

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

void interruptRoutine()
{
  // Just set the flag that we have updated data and return from the ISR
  dataReady = true;
}
