#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>

Adafruit_LSM303 lsm;

void setup()
{
#ifndef ESP8266
  while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
#endif
  Serial.begin(9600);

  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
    while (1);
  }
}

void loop()
{
  lsm.read();
  Serial.print("AX: "); Serial.print((int)lsm.accelData.x); Serial.print(" ");
  Serial.print("AY: "); Serial.print((int)lsm.accelData.y); Serial.print(" ");
  Serial.print("AZ: "); Serial.print((int)lsm.accelData.z); Serial.print(" ");
  Serial.print("MX: "); Serial.print((int)lsm.magData.x);   Serial.print(" ");
  Serial.print("MY: "); Serial.print((int)lsm.magData.y);   Serial.print(" ");
  Serial.print("MZ: "); Serial.println((int)lsm.magData.z); Serial.print(" ");
  delay(100);
}
