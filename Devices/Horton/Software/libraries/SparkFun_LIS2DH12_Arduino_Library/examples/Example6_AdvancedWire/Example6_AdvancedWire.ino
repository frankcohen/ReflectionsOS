/*
  Reading and controlling the very low power LIS2DH12
  Author: Nathan Seidle
  Created: Septempter 18th, 2019
  License: This code is Lemonadeware; do whatever you want with this code.
  If you see me (or any other SparkFun employee) at the
  local, and you've found our code helpful, please buy us a round!

  This library accepts a device address and wire port so you can use this
  library with Wire1, Wire2, etc and 0x19 or 0x18 for the device address.

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  Edge: https://www.sparkfun.com/products/15170
  Edge 2: https://www.sparkfun.com/products/15420
  Qwiic LIS2DH12 Breakout: https://www.sparkfun.com/products/15760

  Hardware Connections:
  Plug a Qwiic cable into the Qwiic Accelerometer RedBoard Qwiic or BlackBoard
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
*/

#include <Wire.h>

#include "SparkFun_LIS2DH12.h" //Click here to get the library: http://librarymanager/All#SparkFun_LIS2DH12

SPARKFUN_LIS2DH12 accel; //Create instance

#define ACCEL_ADDRESS 0x19 //Default address on the LIS2DH12 breakout board, Edge, and Edge2
//#define ACCEL_ADDRESS 0x18 //Address if the address jumper is closed on the breakout board

void setup()
{
  Serial.begin(115200);
  Serial.println("SparkFun Accel Example");

  Wire1.begin(); //Compilation will fail here if your platform doesn't have multiple Wire ports

  if (accel.begin(ACCEL_ADDRESS, Wire1) == false)
  {
    Serial.println("Accelerometer not detected. Are you sure you did a Wire1.begin()? Freezing...");
    while (1);
  }
}

void loop()
{
  //Print accel values only if new data is available
  if (accel.available())
  {
    float accelX = accel.getX();
    float accelY = accel.getY();
    float accelZ = accel.getZ();
    float tempC = accel.getTemperature();

    Serial.print("Acc [mg]: ");
    Serial.print(accelX, 1);
    Serial.print(" x, ");
    Serial.print(accelY, 1);
    Serial.print(" y, ");
    Serial.print(accelZ, 1);
    Serial.print(" z, ");
    Serial.print(tempC, 1);
    Serial.print("C");
    Serial.println();
  }
}
