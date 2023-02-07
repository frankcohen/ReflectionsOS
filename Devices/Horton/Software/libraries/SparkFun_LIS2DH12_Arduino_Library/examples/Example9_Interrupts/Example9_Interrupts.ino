/*
  Reading and controlling the very low power LIS2DH12
  Author: Nathan Seidle
  Created: April 15th, 2022
  License: This code is Lemonadeware; do whatever you want with this code.
  If you see me (or any other SparkFun employee) at the
  local, and you've found our code helpful, please buy us a round!

  This example demonstrates how to enable and read interrupts from the LIS2DH12

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
SPARKFUN_LIS2DH12 accel;

int accelInterruptPin = 4;

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("SparkFun Accel Example");

  Wire.begin();

  if (accel.begin() == false)
  {
    Serial.println("Accelerometer not detected. Check address jumper and wiring. Freezing...");
    while (1)
      ;
  }

  //Configure the LIS2DH12 to pull pin low when it is tilted above a certain angle

  pinMode(accelInterruptPin, INPUT_PULLUP);

  accel.setDataRate(LIS2DH12_POWER_DOWN); //Stop measurements

  //INT1_CFG - enable X and Y events
  //accel.setInt1(true);
  accel.setInt1(false);

  //Set INT POLARITY to Active Low
  //CTRL_REG6, INT_POLARITY = 1 active low
  accel.setIntPolarity(LOW);

  //Set INT1 interrupt
  //CTRL_REG3, I1_IA1 = 1 - Enable IA1 Interrupt on INT1 pin
  accel.setInt1IA1(true);

  //Set INT1 threshold
  //INT1_THS = 500mg / 16mb = 31
  //accel.setInt1Threshold(62); //90 degree tilt before interrupt
  accel.setInt1Threshold(31); //45 degree tilt before interrupt
  //accel.setInt1Threshold(10);

  //Set INT1 Duration
  //INT1_DURATION = 500
  //accel.setInt1Duration(30);
  accel.setInt1Duration(9);

  //Latch interrupt 1, CTRL_REG5, LIR_INT1
  //accel.setInt1Latch(true);
  accel.setInt1Latch(false);

  //Clear the interrupt
  while (accel.getInt1()) delay(10); //Reading int will clear it

  //accel.setDataRate(LIS2DH12_ODR_1Hz); //Very low power
  //accel.setDataRate(LIS2DH12_ODR_100Hz);
  accel.setDataRate(LIS2DH12_ODR_400Hz); 

  accel.setInt1(true); //Enable interrupts

  Serial.println("Begin Interrupt Scanning");
  Serial.println("Tilt the board to 45 degrees or more");
}

void loop()
{
  //We can either check the pin
  if (digitalRead(accelInterruptPin) == LOW)
  {
    Serial.println("Accel int pin is low!");
  }

  //Or we can poll for the interrupt via I2C
  if (accel.getInt1() == true) //Reading int will clear it
  {
    Serial.println("Software int!");
  }

  Serial.print(".");
  delay(100);
}
