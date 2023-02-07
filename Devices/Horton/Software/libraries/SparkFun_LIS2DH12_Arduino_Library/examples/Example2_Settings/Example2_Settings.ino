/*
  Reading and controlling the very low power LIS2DH12
  Author: Nathan Seidle
  Created: Septempter 18th, 2019
  License: This code is Lemonadeware; do whatever you want with this code.
  If you see me (or any other SparkFun employee) at the
  local, and you've found our code helpful, please buy us a round!

  By default, sensor is set to 2g, 25Hz, 12bit resolution when .begin() is run.
  This example demonstrates how to configure the scale, data rate, and
  reading resolution. 

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
SPARKFUN_LIS2DH12 accel;       //Create instance

void setup()
{
  Serial.begin(115200);
  Serial.println("SparkFun Accel Example");

  Wire.begin();

  if (accel.begin() == false)
  {
    Serial.println("Accelerometer not detected. Freezing...");
    while (1)
      ;
  }

  accel.setScale(LIS2DH12_2g);
  //accel.setScale(LIS2DH12_4g);
  //accel.setScale(LIS2DH12_8g);
  //accel.setScale(LIS2DH12_16g);
  int currentScale = accel.getScale();
  Serial.print("Current scale (0 to 3): ");
  Serial.println(currentScale);

  //accel.setMode(LIS2DH12_LP_8bit);
  //accel.setMode(LIS2DH12_NM_10bit);
  accel.setMode(LIS2DH12_HR_12bit);
  int currentMode = accel.getMode();
  Serial.print("Current mode (0 to 2): ");
  Serial.println(currentMode);

  //Sensor output rate ranges from 1 to 5.376kHz
  //The lower the output rate the lower the power consumption
  //accel.setDataRate(LIS2DH12_POWER_DOWN);
  //accel.setDataRate(LIS2DH12_ODR_1Hz);
  accel.setDataRate(LIS2DH12_ODR_10Hz);
  //accel.setDataRate(LIS2DH12_ODR_25Hz);
  //accel.setDataRate(LIS2DH12_ODR_50Hz);
  //accel.setDataRate(LIS2DH12_ODR_100Hz);
  //accel.setDataRate(LIS2DH12_ODR_200Hz);
  //accel.setDataRate(LIS2DH12_ODR_400Hz);
  //accel.setDataRate(LIS2DH12_ODR_1kHz620_LP);
  //accel.setDataRate(LIS2DH12_ODR_5kHz376_LP_1kHz344_NM_HP);
  int currentRate = accel.getDataRate();
  Serial.print("Current rate (0 to 9): ");
  Serial.println(currentRate);
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
