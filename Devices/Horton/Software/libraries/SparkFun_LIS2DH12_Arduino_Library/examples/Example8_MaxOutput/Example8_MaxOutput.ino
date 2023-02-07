/*
  Reading and controlling the very low power LIS2DH12
  Author: Nathan Seidle
  Created: Septempter 18th, 2019
  License: This code is Lemonadeware; do whatever you want with this code.
  If you see me (or any other SparkFun employee) at the
  local, and you've found our code helpful, please buy us a round!

  Read XYZ at maximum 5kHz rate. Note: This requires the serial output to increase
  to 500kbps and I2C to 1MHz. The highest rate we've seen is ~900Hz over I2C.

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
  Wire.setClock(1000000);

  if (accel.begin() == false)
  {
    Serial.println("Accelerometer not detected. Freezing...");
    while (1)
      ;
  }

  accel.setScale(LIS2DH12_2g);

  //accel.setMode(LIS2DH12_LP_8bit);
  //accel.setMode(LIS2DH12_NM_10bit);
  accel.setMode(LIS2DH12_HR_12bit);

  //accel.setDataRate(LIS2DH12_ODR_1kHz620_LP);
  accel.setDataRate(LIS2DH12_ODR_5kHz376_LP_1kHz344_NM_HP);
  accel.disableTemperature();
}

long measurements = 0;

void loop()
{
  if (accel.available())
  {
    int accelX = accel.getRawX();
    int accelY = accel.getRawY();
    int accelZ = accel.getRawZ();
    //    //float tempC = accel.getTemperature();
    //
    //    Serial.print("x:");
    //    Serial.print(accelX);
    //    Serial.print(" y:");
    //    Serial.print(accelY);
    //    Serial.print(" z:");
    //    Serial.print(accelZ);
    //    //    Serial.print(tempC, 1);
    //    //    Serial.print("C");
    //
    //    Serial.print(" hz:");
    //    //Serial.print(measurements / (millis() / 1000.0), 2);
    //    Serial.println();

    measurements++;

    if (measurements % 1000 == 0)
    {
      int hertz = measurements * 1000 / millis();
      Serial.print("measurements: ");
      Serial.print(measurements);
      Serial.print(" time: ");
      Serial.print(millis());
      Serial.print(" Measurement rate: ");
      Serial.print(hertz);
      Serial.print("Hz");
      Serial.println();
    }
  }

  //delay(10);
}