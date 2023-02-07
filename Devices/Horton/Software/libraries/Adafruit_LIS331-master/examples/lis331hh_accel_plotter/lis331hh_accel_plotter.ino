
// A plotter-friendly basic demo for accelerometer readings from the Adafruit LIS331HH breakout

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>

// Used for software SPI
#define LIS331HH_CLK 13
#define LIS331HH_MISO 12
#define LIS331HH_MOSI 11
// Used for hardware & software SPI
#define LIS331HH_CS 10

// software SPI
//Adafruit_LIS331HH lis = Adafruit_LIS331HH(LIS331HH_CS, LIS331HH_MOSI, LIS331HH_MISO, LIS331HH_CLK);
// hardware SPI
//Adafruit_LIS331HH lis = Adafruit_LIS331HH(LIS331HH_CS);
// I2C
Adafruit_LIS331HH lis = Adafruit_LIS331HH();

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  if (! lis.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  lis.setRange(LIS331HH_RANGE_12_G);   // 6, 12, or 24 G
  lis.setDataRate(LIS331_DATARATE_1000_HZ);
}

void loop() {
  /* Get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results as m/s^2 in a plotter-friendly format */
  Serial.print(event.acceleration.x);
  Serial.print(","); Serial.print(event.acceleration.y);
  Serial.print(","); Serial.print(event.acceleration.z);
  Serial.println("");

  delay(1);
}
