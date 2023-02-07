
/* Demo of the filter methods for the Adafruit H3LIS331 and LIS331HH sensor breakouts
Prints plotter formatted  measurements before and after having the filters applied. Multiple options are available, comment out and uncomment different options and re-upload the sketch to see their effect.

**Note** The higher the range, the more noise the sinal will have. Additionally the H3LIS331DL is _especially_ noisy.
*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_H3LIS331.h>
#include <Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>

bool waiting_to_reset;
//Adafruit_H3LIS331 lis = Adafruit_H3LIS331();
Adafruit_LIS331HH lis = Adafruit_LIS331HH();

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

 if (! lis.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  // lis.setRange(H3LIS331_RANGE_100_G); // For the H3LIS331
  lis.setRange(LIS331HH_RANGE_6_G); // For the LIS331HH

  // lis.setDataRate(LIS331_DATARATE_LOWPOWER_10_HZ); // Use to test the low-pass filter
  lis.setDataRate(LIS331_DATARATE_1000_HZ); // Use the highest data rate for a higher resolution signal

  // lis.setLPFCutoff(LIS331_LPF_37_HZ); // requires that DataRate is a LOW_POWER rate
  lis.enableHighPassFilter(true, LIS331_HPF_0_0025_ODR); //0.125Hz Cutoff

//  lis.enableHighPassFilter(true, LIS331_HPF_0_0025_ODR, true); // use the reference
//  lis.setHPFReference(127); // value is signed from -127 to 127 for negative and positive offsets
//  Serial.print("HPF reference: ");Serial.println(lis.getHPFReference());

}

void loop() {
  measure_n_times(255);
  measure_n_times(255);
  measure_n_times(255);
  measure_n_times(255);
  // lis.HPFReset(); // instantly zeros the mesurements, avoiding the "warm up" time of the High-pass filter
  measure_n_times(255);
  lis.enableHighPassFilter(false);
  measure_n_times(255);
  lis.enableHighPassFilter(true, LIS331_HPF_0_0025_ODR);
}

void measure_n_times(uint8_t count){
  for(int i=0; i< count; i++){
    sensors_event_t event;
    lis.getEvent(&event);
    Serial.println(event.acceleration.z);
    delayMicroseconds(1000);
  }
}
