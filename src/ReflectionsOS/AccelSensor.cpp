/*
  Reflections is a hardware and software platform for building entertaining mobile experiences.

  Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS/)
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Reflections board usees an (LIS3DHTR 3-Axis Accelerometer) to
  identify user gestures with their wrists and to wake the
  processor from sleep.

  Recognizes gestures by moving the Reflections board

  See https://github.com/frankcohen/ReflectionsOS/Experiments/AccelerometerLab/ReadMe.md

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Depends on these libraries:
  Adafruit LIS3DH library, https://github.com/adafruit/Adafruit_LIS3DH
  Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
  Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
  PeakDetection, peak and valley detection, https://github.com/leandcesar/PeakDetection

  Datasheet comes with this source code, see:
  1811031937_STMicroelectronics-LIS3DHTR_C15134 Accelerometer.pdf

Sensor and support library details:

getClick() bit definitions:
Bit	Name	   Description
6	  IA	     Interrupt Active: Set to 1 when a click event is detected.
5	  DCLICK	 Double Click: Set to 1 if a double click event is detected.
4	  SCLICK	 Single Click: Set to 1 if a single click event is detected.
3	  Sign	   Sign of Acceleration: Indicates the direction of the click event.
2	  Z	Z-Axis Contribution: Set to 1 if the Z-axis contributed to the click.
1	  Y	Y-Axis Contribution: Set to 1 if the Y-axis contributed to the click.
0	  X	X-Axis Contribution: Set to 1 if the X-axis contributed to the click.

Scaling factor for different ranges:
For LIS3DH_RANGE_2_G (±2g), the scaling factor is 0.001 g per count.
For LIS3DH_RANGE_4_G (±4g), the scaling factor is 0.002 g per count.
For LIS3DH_RANGE_8_G (±8g), the scaling factor is 0.004 g per count.
For LIS3DH_RANGE_16_G (±16g), the scaling factor is 0.008 g per count.

Data rate for different applications:
Low-speed movements (walking): 10–50 Hz., 100 ms delay between reads
Medium-speed movements (hand gestures): 50–100 Hz., 10 ms delay between reads
High-speed movements (vibrations, impacts): 200–400 Hz., 5 ms or less between reads

getClick() bit definitions:
Bit	Name	   Description
6	  IA	     Interrupt Active: Set to 1 when a click event is detected.
5	  DCLICK	 Double Click: Set to 1 if a double click event is detected.
4	  SCLICK	 Single Click: Set to 1 if a single click event is detected.
3	  Sign	   Sign of Acceleration: Indicates the direction of the click event.
2	  Z	Z-Axis Contribution: Set to 1 if the Z-axis contributed to the click.
1	  Y	Y-Axis Contribution: Set to 1 if the Y-axis contributed to the click.
0	  X	X-Axis Contribution: Set to 1 if the X-axis contributed to the click.

Learn how to wake an ESP32-S3 from deep sleep with interrupt movement sensing at 
https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Deep%20Sleep/Deep_Sleep.md

*/

#include "AccelSensor.h"

Adafruit_LIS3DH lis3dh = Adafruit_LIS3DH();

AccelSensor::AccelSensor(){}

void AccelSensor::begin()
{ 
  started = false;
  runflag = false;

  aboveThreshold = false;
  lastDetectedTime = 0;
  singleCount = 0;
  doubleCount = 0;
  tripleCount = 0;
  lastResetTime = 0;
  lastSummaryTime = 0;
  lastMag = 0;
  lastStdDev = 0;
  lastThreshold = 0;
  tapHist[0] = 0;
  tapHist[1] = 0;
  tapHist[2] = 0;

  if ( ! lis3dh.begin( accelAddress ) ) 
  {
    Serial.println( F( "Accelerometer did not start, stopping" ) );
    while (1) yield();
  }

  // Set shake detection parameters
  lis3dh.setRange( range );   // 2, 4, 8 or 16 G!

  lis3dh.setDataRate( datarate ); // Set ODR to 100 Hz

  // bootstrap the EMA with one reading
  sensors_event_t evt;
  lis3dh.getEvent(&evt);
  float m = magnitude(evt);
  emaMean = m;
  emaVar  = 1.0f;

  lastResetTime   = millis();
  lastSummaryTime = millis();

  enableWakeOnMotion();

  started = true;
}

float AccelSensor::getXreading() 
{
  if ( started )
  {
    lis3dh.read();
    return lis3dh.x; // Raw X value
  }
  else
  {
    return 0;
  }
}

float AccelSensor::getYreading() 
{
  if ( started )
  {
    lis3dh.read();
    return lis3dh.y; // Raw Y value
  }
  else
  {
    return 0;
  }
}

float AccelSensor::getZreading() 
{
  if ( started )
  {
    lis3dh.read();
    return lis3dh.z; // Raw Y value
  }
  else
  {
    return 0;
  }
}

// Resets the sensor

void AccelSensor::resetLIS3DH() 
{
  // Read the current value of CTRL_REG5 (0x24)
  uint8_t ctrlReg5Value = 0;
  Wire.beginTransmission(0x18);  // Start communication with LIS3DH (I2C address)
  Wire.write(0x24);  // Register address for CTRL_REG5
  Wire.endTransmission(false);  // Keep the connection active
  Wire.requestFrom(0x18, 1);  // Request 1 byte of data
  if (Wire.available()) {
    ctrlReg5Value = Wire.read();  // Read the current value from CTRL_REG5
  }

  // Set the REBOOT_MEMORY bit (bit 7) to reset the device
  ctrlReg5Value |= 0x80;  // Set the most significant bit to 1 (0x80)

  // Write the new value back to CTRL_REG5 to reboot the sensor
  Wire.beginTransmission(0x18);  // Start communication again
  Wire.write(0x24);  // CTRL_REG5 register address
  Wire.write(ctrlReg5Value);  // Write the updated value
  Wire.endTransmission();  // End transmission

  // Wait for the reset to take effect
  delay(10);
}

// Helpers to read/write a single LIS3DH register over I²C
static uint8_t lis3dh_read8(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, (uint8_t)1);
  return Wire.read();
}

static void lis3dh_write8(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

// Configure to wake from deep sleep on movement

void AccelSensor::enableWakeOnMotion() 
{
  // 0) make sure Wire is up (lis3dh.begin() usually calls Wire.begin())
  //    and that accelAddress matches your I2C address (0x18 or 0x19)

  // 1) Enable the high-pass filter on CTRL_REG2 (e.g. FDS bit)
  uint8_t ctrl2 = lis3dh_read8(accelAddress, LIS3DH_REG_CTRL2);
  ctrl2 |= 0x08;  // same mask you had: bit-3 = filtered-data-selection
  lis3dh_write8(accelAddress, LIS3DH_REG_CTRL2, ctrl2);

  // 2) Build INT1_CFG for “wake on motion” (ZH, YH and XH high)
  //    [AOI=0, 6D=0, ZHIE=1, ZLIE=0, YHIE=1, YLIE=0, XHIE=1, XLIE=0]
  uint8_t int1cfg = (1<<5) | (1<<3) | (1<<1);
  lis3dh_write8(accelAddress, LIS3DH_REG_INT1CFG, int1cfg);

  // 3) Set the threshold (1–127) and duration (1–127) on INT1
  lis3dh_write8(accelAddress, LIS3DH_REG_INT1THS, 16);
  lis3dh_write8(accelAddress, LIS3DH_REG_INT1DUR, 10);

  // 4) Latch the interrupt on INT1 (so it stays asserted until you clear it)
  uint8_t ctrl5 = lis3dh_read8(accelAddress, LIS3DH_REG_CTRL5);
  ctrl5 |= (1<<3); // LIR_INT1 = bit-3 in CTRL_REG5
  lis3dh_write8(accelAddress, LIS3DH_REG_CTRL5, ctrl5);

  // 5) Finally, tell the ESP32 “any high on GPIO_NUM_14 (your INT1 pin)
  //    will wake me from deep sleep”
  esp_sleep_enable_ext1_wakeup(
    BIT(GPIO_NUM_14),
    ESP_EXT1_WAKEUP_ANY_HIGH
  );
}

/* Enabled wake on movement using SparkFunLIS3DH library

  uint8_t ctrlReg2;
  lis3dh.readRegister(&ctrlReg2, LIS3DH_CTRL_REG2);
  ctrlReg2 |= 0x08; // Enable high-pass filter
  lis3dh.writeRegister(LIS3DH_CTRL_REG2, ctrlReg2);

  lis3dh.writeRegister(LIS3DH_INT1_THS, 16 );        // threshold is 1 - 127
  lis3dh.writeRegister(LIS3DH_INT1_DURATION, 10 );   // interrupt duration 1 - 127 (increase to make it less sensitive)

  uint8_t dataToWrite = 0;

  //LIS3DH_INT1_CFG   
  //dataToWrite |= 0x80;//AOI, 0 = OR 1 = AND
  //dataToWrite |= 0x40;//6D, 0 = interrupt source, 1 = 6 direction source
  //Set these to enable individual axes of generation source (or direction)
  // -- high and low are used generically
  dataToWrite |= 0x20;//Z high
  //dataToWrite |= 0x10;//Z low
  dataToWrite |= 0x08;//Y high
  //dataToWrite |= 0x04;//Y low
  dataToWrite |= 0x02;//X high
  //dataToWrite |= 0x01;//X low
  lis3dh.writeRegister(LIS3DH_INT2_CFG, dataToWrite);
  
  //LIS3DH_INT1_THS   
  dataToWrite = 0;
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x10; // 1/8 range
  lis3dh.writeRegister(LIS3DH_INT2_THS, dataToWrite);
  
  //LIS3DH_INT1_DURATION  
  dataToWrite = 1;
  //minimum duration of the interrupt
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x01; // 1 * 1/50 s = 20ms
  lis3dh.writeRegister(LIS3DH_INT2_DURATION, dataToWrite);
  
  //LIS3DH_CLICK_CFG   
  dataToWrite = 0;
  //Set these to enable individual axes of generation source (or direction)
  // -- set = 1 to enable
  //dataToWrite |= 0x20;//Z double-click
  dataToWrite |= 0x10;//Z click
  //dataToWrite |= 0x08;//Y double-click 
  dataToWrite |= 0x04;//Y click
  //dataToWrite |= 0x02;//X double-click
  dataToWrite |= 0x01;//X click
  lis3dh.writeRegister(LIS3DH_CLICK_CFG, dataToWrite);
  
  //LIS3DH_CLICK_SRC
  dataToWrite = 0;
  //Set these to enable click behaviors (also read to check status)
  // -- set = 1 to enable
  //dataToWrite |= 0x20;//Enable double clicks
  dataToWrite |= 0x04;//Enable single clicks
  //dataToWrite |= 0x08;//sine (0 is positive, 1 is negative)
  dataToWrite |= 0x04;//Z click detect enabled
  dataToWrite |= 0x02;//Y click detect enabled
  dataToWrite |= 0x01;//X click detect enabled
  lis3dh.writeRegister(LIS3DH_CLICK_SRC, dataToWrite);
  
  //LIS3DH_CLICK_THS   
  dataToWrite = 0;
  //This sets the threshold where the click detection process is activated.
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x0A; // ~1/16 range
  lis3dh.writeRegister(LIS3DH_CLICK_THS, dataToWrite);
  
  //LIS3DH_TIME_LIMIT  
  dataToWrite = 0;
  //Time acceleration has to fall below threshold for a valid click.
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x08; // 8 * 1/50 s = 160ms
  lis3dh.writeRegister(LIS3DH_TIME_LIMIT, dataToWrite);
  
  //LIS3DH_TIME_LATENCY
  dataToWrite = 0;
  //hold-off time before allowing detection after click event
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x08; // 4 * 1/50 s = 160ms
  lis3dh.writeRegister(LIS3DH_TIME_LATENCY, dataToWrite);
  
  //LIS3DH_TIME_WINDOW 
  dataToWrite = 0;
  //hold-off time before allowing detection after click event
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x10; // 16 * 1/50 s = 320ms
  lis3dh.writeRegister(LIS3DH_TIME_WINDOW, dataToWrite);

  //LIS3DH_CTRL_REG5
  //Int1 latch interrupt and 4D on  int1 (preserve fifo en)
  lis3dh.readRegister(&dataToWrite, LIS3DH_CTRL_REG5);
  dataToWrite &= 0xF3; //Clear bits of interest
  dataToWrite |= 0x08; //Latch interrupt (Cleared by reading int1_src)
  //dataToWrite |= 0x04; //Pipe 4D detection from 6D recognition to int1?
  lis3dh.writeRegister(LIS3DH_CTRL_REG5, dataToWrite);

  //LIS3DH_CTRL_REG3
  //Choose source for pin 1
  dataToWrite = 0;
  dataToWrite |= 0x80; //Click detect on pin 1
  //dataToWrite |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
  //dataToWrite |= 0x20; //AOI2 event ()
  //dataToWrite |= 0x10; //Data ready
  //dataToWrite |= 0x04; //FIFO watermark
  //dataToWrite |= 0x02; //FIFO overrun
  lis3dh.writeRegister(LIS3DH_CTRL_REG3, dataToWrite);

  //LIS3DH_CTRL_REG6
  //Choose source for pin 2 and both pin output inversion state
  dataToWrite = 0;
  dataToWrite |= 0x80; //Click int on pin 2
  //dataToWrite |= 0x40; //Generator 1 interrupt on pin 2
  //dataToWrite |= 0x10; //boot status on pin 2
  //dataToWrite |= 0x02; //invert both outputs
  lis3dh.writeRegister(LIS3DH_CTRL_REG6, dataToWrite);

  // Enable ESP32 to wake up on INT2 (GPIO13) high level
  esp_sleep_enable_ext1_wakeup(BIT(GPIO_NUM_14), ESP_EXT1_WAKEUP_ANY_HIGH);
*/

/**
 * Computes the acceleration magnitude and maintains a running mean and variance via an exponential moving average.
 * Sets a dynamic threshold at (mean + thresholdFactor·σ) and detects taps as rising‐edge spikes above that threshold.
 * Classifies single, double, and triple taps by checking if tap timestamps fall within configurable timing windows.
 */

void AccelSensor::dynamicTapDetection()
{
  if ( ! runflag ) return;

  unsigned long now = millis();

  // Read & compute magnitude

  sensors_event_t evt;
  lis3dh.getEvent(&evt);
  float mag = magnitude(evt);

  // Dynamic threshold
  
  float stdDev    = sqrtf(emaVar);
  float threshold = emaMean + _thresholdFactor * stdDev;

  // Store debug stats

  lastMag       = mag;
  lastStdDev    = stdDev;
  lastThreshold = threshold;

  // Rising-edge with debounce

  if (mag > threshold
      && !aboveThreshold
      && (now - lastDetectedTime > _minTapInterval)) {
      aboveThreshold   = true;
      lastDetectedTime = now;
      handleTap(now);
  } else if (mag <= threshold) {
      aboveThreshold = false;
      // Update EMA
      float prevMean = emaMean;
      emaMean = _alpha * mag + (1 - _alpha) * emaMean;
      emaVar  = _alpha * (mag - prevMean)*(mag - prevMean) + (1 - _alpha) * emaVar;
  }

  // Reset counts periodically

    if (now - lastResetTime >= _resetInterval) 
    {
        singleCount = doubleCount = tripleCount = 0;
        _pendingSingle = _pendingDouble = _pendingTriple = false;
        _singleTime = _doubleTime = _tripleTime = 0;
        lastResetTime = now;
    }

/*
    if (now - lastSummaryTime >= _summaryInterval) {
        Serial.print("Singles: "); Serial.print(singleCount);
        Serial.print("  Doubles: "); Serial.print(doubleCount);
        Serial.print("  Triples: "); Serial.print(tripleCount);
        Serial.print("  Mag:");    Serial.print(lastMag, 2);
        Serial.print("  Mean:");   Serial.print(emaMean, 2);
        Serial.print("  σ:");      Serial.print(lastStdDev, 2);
        Serial.print("  Thr:");    Serial.println(lastThreshold, 2);
        lastSummaryTime = now;
    }
*/

}

void AccelSensor::handleTap(unsigned long now) 
{
  // Shift history
  tapHist[2] = tapHist[1];
  tapHist[1] = tapHist[0];
  tapHist[0] = now;

  // Debug timestamps
  Serial.print("Hist: [");
  Serial.print(tapHist[2]); Serial.print(", ");
  Serial.print(tapHist[1]); Serial.print(", ");
  Serial.print(tapHist[0]); Serial.print("]  Δ1:");
  Serial.print(tapHist[0] - tapHist[1]);
  Serial.print("  Δ2:"); Serial.println(tapHist[1] - tapHist[2]);

// Triple tap
  if (tapHist[2] != 0 && (tapHist[0] - tapHist[2] <= _tripleWindow)) 
  {
    tripleCount++;
    Serial.println(">>> Triple tap!");
    _pendingTriple = true;
    _tripleTime   = now;
    _pendingDouble = _pendingSingle = false;
  }
  // Double tap
  else if (tapHist[1] != 0 && (tapHist[0] - tapHist[1] <= _doubleWindow)) 
  {
    doubleCount++;
    Serial.println(">> Double tap!");
    _pendingDouble = true;
    _doubleTime    = now;
    _pendingSingle = false;
  }
  // Single tap
  else 
  {
    singleCount++;
    Serial.println("> Single tap");
    _pendingSingle = true;
    _singleTime    = now;
  }
}

void AccelSensor::reset()
{
  _pendingSingle = false;
  _pendingDouble = false;
  _pendingTriple = false;
}

bool AccelSensor::getSingleTap() 
{
  // Fires after the double-window elapses
  if (_pendingSingle && (millis() - _singleTime >= _doubleWindow)) 
  {
    _pendingSingle = false;
    return true;
  }
  return false;
}

bool AccelSensor::getDoubleTap() 
{
  // Fires after the triple-window elapses
  if (_pendingDouble && (millis() - _doubleTime >= _tripleWindow)) 
  {
    _pendingDouble = false;
    return true;
  }
  return false;
}

bool AccelSensor::getTripleTap() 
{
  // Fires immediately upon triple detection
  if (_pendingTriple) 
  {
    _pendingTriple = false;
    return true;
  }
  return false;
}

float AccelSensor::magnitude(const sensors_event_t &e) 
{
  return sqrtf(
      e.acceleration.x*e.acceleration.x +
      e.acceleration.y*e.acceleration.y +
      e.acceleration.z*e.acceleration.z
  );
}

/* Used to send debug information to the Serial Monitor
   TOF operates in Core 0 in parallel to the Arduino code
   running the Serial Monitor in Core 1. Using Serial.println( F("hi") )
   will often be clipped or ignored when run in Core 0. The main
   loop() in ReflectionsOS.ino calls TOF::getMef() and prints to
   Serial Monitor from there. */

String AccelSensor::getRecentMessage()
{
  String myMefa = myMef;
  myMef = "";
  return myMefa;
}

String AccelSensor::getRecentMessage2()
{
  String myMefa = myMef2;
  myMef2 = "";
  return myMefa;
}

void AccelSensor::setStatus( bool run )
{
  runflag = run;
}

bool AccelSensor::getStatus()
{
  return runflag;
}


void AccelSensor::loop()
{
  dynamicTapDetection();
}

