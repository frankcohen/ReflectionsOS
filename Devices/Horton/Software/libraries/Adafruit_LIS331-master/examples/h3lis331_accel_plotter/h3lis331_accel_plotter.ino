
// A plotter-friendly basic demo for accelerometer readings from the Adafruit H3LIS331 breakout

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_H3LIS331.h>
#include <Adafruit_Sensor.h>

// Used for software SPI
#define H3LIS331_CLK 13
#define H3LIS331_MISO 12
#define H3LIS331_MOSI 11
// Used for hardware & software SPI
#define H3LIS331_CS 10

// software SPI
//Adafruit_H3LIS331 lis = Adafruit_H3LIS331(H3LIS331_CS, H3LIS331_MOSI, H3LIS331_MISO, H3LIS331_CLK);
// hardware SPI
//Adafruit_H3LIS331 lis = Adafruit_H3LIS331(H3LIS331_CS);
// I2C
Adafruit_H3LIS331 lis = Adafruit_H3LIS331();

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  if (! lis.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  lis.setRange(H3LIS331_RANGE_100_G);   // 6, 12, or 24 G
  lis.setDataRate(LIS331_DATARATE_1000_HZ);
}

void loop() {
  lis.read();      // get X Y and Z data at once

  /* Or....get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);
//
  /* Display the results as m/s^2 in a plotter-friendly format */
  Serial.print(event.acceleration.x);
  Serial.print(","); Serial.print(event.acceleration.y);
  Serial.print(","); Serial.print(event.acceleration.z);
  Serial.println();
  /* Alternately, given the range of the H3LIS331, display the results measured in g in a plotter-friendly format */
//  Serial.print(event.acceleration.x/SENSORS_GRAVITY_STANDARD);
//  Serial.print(","); Serial.print(event.acceleration.y/SENSORS_GRAVITY_STANDARD);
//  Serial.print(","); Serial.print(event.acceleration.z/SENSORS_GRAVITY_STANDARD);
//  Serial.println();

  delay(1);
}
