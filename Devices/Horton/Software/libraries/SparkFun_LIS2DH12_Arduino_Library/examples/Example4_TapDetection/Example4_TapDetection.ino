/* 
  Reading and controlling the very low power LIS2DH12
  Author: Nathan Seidle
  Created: Septempter 18th, 2019
  License: This code is Lemonadeware; do whatever you want with this code.
  If you see me (or any other SparkFun employee) at the
  local, and you've found our code helpful, please buy us a round!
  
  This example demonstrates how to read taps on the LIS2DH12.

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

int tapCounter = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("SparkFun Accel Example");

  Wire.begin();

  if (accel.begin() == false)
  {
    Serial.println("Accelerometer not detected. Are you sure you did a Wire1.begin()? Freezing...");
    while (1)
      ;
  }

  //By default, sensor is set to 25Hz, 12bit, 2g at .begin()
  //When these are changed, tap threshold may need to be altered

  accel.enableTapDetection();
  accel.setTapThreshold(40); //7-bit value. Max value is 127. 10 is a little low. 40 is ok. 100 is a bit hard.

  while (accel.isTapped() == true)
    delay(10); //Clear any initial event that may be in the buffer
}

void loop()
{
  if (accel.isTapped())
  {
    Serial.print("Tap: ");
    Serial.println(++tapCounter);

    while (accel.isTapped() == true)
      delay(10); //Wait for event to complete
  }
}
