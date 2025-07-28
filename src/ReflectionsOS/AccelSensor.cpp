/*
  Reflections is a hardware and software platform for building entertaining mobile experiences.

  Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS/)
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.
 
  This module performs real-time detection of single and double tap gestures 
  using the LIS3DH accelerometer. It includes two distinct phases:
 
  1. Calibration and Validation Phase (begin):
     • Processes a static array of predefined acceleration values.
     • Calibrates an EMA-based (Exponential Moving Average) baseline and threshold.
     • Validates the detection logic by running a state machine over sample data, 
       outputting "single" and "double" tap events for confirmation.
 
  2. Live Detection Phase (loop):
     • Continuously reads acceleration values from the LIS3DH sensor in m/s².
     • Applies the same EMA threshold logic used in begin.
     • Runs a state machine to detect live single and double tap gestures based on:
          - Dynamic deltas above an adaptive threshold
          - Time intervals between candidate taps
          - Suppression of noise using temporal and amplitude filters
  
  Design Goals:
   • Accurate single and double tap recognition in real-world usage
   • Noise tolerance via EMA and temporal filtering
   • Portable to other ReflectionsOS gesture-driven applications
  
  Dependencies:
  Adafruit LIS3DH library, https://github.com/adafruit/Adafruit_LIS3DH
  Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
  Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
  
  Datasheet comes with this source code, see:
  1811031937_STMicroelectronics-LIS3DHTR_C15134 Accelerometer.pdf

  Author: Frank Cohen <fcohen@starlingwatch.com>
  License: GPL v3 (See license.txt)
  Project: ReflectionsOS – Mobile Connected Entertainment Platform

  Notes on working with the LIS3DH sensor:

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

AccelSensor::AccelSensor() {}

void AccelSensor::begin() {
  started = false;
  runflag = false;

  lis = Adafruit_LIS3DH();

  if (!lis.begin(accelAddress)) {
    Serial.println(F("Accelerometer did not start, stopping"));
     video.stopOnError( "Accel", "did not", "start", " ", " " );
  }

  // configure sensor for single‐click detection on Z:
  lis.setRange(LIS3DH_RANGE_8_G);
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);
  lis.setClick(1, 20, 10, 0, 255);

  magtime = millis();
  last = millis();

  started = false;
  runflag = false;

  firstpart = false;
  firstClickTime = millis();
  minClickTime = millis();
  waittime = millis();
  _pendingSingle = false;
  _pendingDouble = false;

  shakentime = millis();
  shakentime2 = millis();
  shakencount = 0;

  configureSensorWakeOnMotion();

  started = true;
}

bool AccelSensor::isShaken()
{
  if ( shakencount > SHAKEN_COUNT )
  {
    shakencount = 0;
    return true;
  }

  return false;
}

float AccelSensor::getXreading() {
  if (started) {
    sensors_event_t evt;
    lis.getEvent(&evt);
    return evt.acceleration.x;
  } else {
    return 0;
  }
}

float AccelSensor::getYreading() {
  if (started) {
    sensors_event_t evt;
    lis.getEvent(&evt);
    return evt.acceleration.y;
  } else {
    return 0;
  }
}

float AccelSensor::getZreading() {
  if (started) {
    sensors_event_t evt;
    lis.getEvent(&evt);
    return evt.acceleration.z;
  } else {
    return 0;
  }
}

// ——— register names from Adafruit_LIS3DH.h ———
//  INT1CFG: enable X/Y/Z high-event interrupts on INT1
//  INT1THS: threshold
//  INT1DUR: duration
//  CTRL_REG5: interrupt latch enable
// ————————————————————————————————
  
// raw write to any LIS3DH register over I²C
static void write8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ACCEL_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Configure to wake from deep sleep on a click/tap movement

void AccelSensor::configureSensorWakeOnMotion() {
  // 1) Choose ±8 g (click detection works best at lower ranges)
  lis.setRange(LIS3DH_RANGE_8_G);

  // 2) Configure “single‐click” on all three axes, threshold=0x10 (tune up for a harder tap)
  lis.setClick(1, CLICKTHRESHHOLD);

  // 3) Route click interrupt to INT1 pin (CTRL_REG3 bit7 = I1_CLICK)
  write8(LIS3DH_REG_CTRL3, 0x80);

  // 4) Clear any pending click (read the click‐source register)
  lis.getClick();

  // 5) Arm the ESP32-S3 EXT1 wake on a HIGH at GPIO14 only
  uint64_t wakeMask = (1ULL << ACCEL_INT1_PIN);
  esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);
}

/* Legacy code, could be handy for future user

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

void AccelSensor::enableWakeOnMotion() {
  // 0) make sure Wire is up (lis3dh.begin() usually calls Wire.begin())
  //    and that accelAddress matches your I2C address (0x18 or 0x19)

  // 1) Enable the high-pass filter on CTRL_REG2 (e.g. FDS bit)
  uint8_t ctrl2 = lis3dh_read8(accelAddress, LIS3DH_REG_CTRL2);
  ctrl2 |= 0x08;  // same mask you had: bit-3 = filtered-data-selection
  lis3dh_write8(accelAddress, LIS3DH_REG_CTRL2, ctrl2);

  // 2) Build INT1_CFG for “wake on motion” (ZH, YH and XH high)
  //    [AOI=0, 6D=0, ZHIE=1, ZLIE=0, YHIE=1, YLIE=0, XHIE=1, XLIE=0]
  uint8_t int1cfg = (1 << 5) | (1 << 3) | (1 << 1);
  lis3dh_write8(accelAddress, LIS3DH_REG_INT1CFG, int1cfg);

  // 3) Set the threshold (1–127) and duration (1–127) on INT1
  lis3dh_write8(accelAddress, LIS3DH_REG_INT1THS, 16);
  lis3dh_write8(accelAddress, LIS3DH_REG_INT1DUR, 10);

  // 4) Latch the interrupt on INT1 (so it stays asserted until you clear it)
  uint8_t ctrl5 = lis3dh_read8(accelAddress, LIS3DH_REG_CTRL5);
  ctrl5 |= (1 << 3);  // LIR_INT1 = bit-3 in CTRL_REG5
  lis3dh_write8(accelAddress, LIS3DH_REG_CTRL5, ctrl5);

  // 5) Finally, tell the ESP32 “any high on GPIO_NUM_14 (your INT1 pin)
  //    will wake me from deep sleep”
  esp_sleep_enable_ext1_wakeup(
    BIT(GPIO_NUM_14),
    ESP_EXT1_WAKEUP_ANY_HIGH);

  // Enable wake on movement using SparkFunLIS3DH library

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

void AccelSensor::resetTaps() {
  _pendingSingle = false;
  _pendingDouble = false;
}

bool AccelSensor::getSingleTapNoClear() {
  return _pendingSingle;
}

bool AccelSensor::getDoubleTapNoClear() {
  return _pendingDouble;
}

bool AccelSensor::getSingleTap() {
  bool ts = _pendingSingle;
  _pendingSingle = false;
  return ts;
}

bool AccelSensor::getDoubleTap() {
  bool ts = _pendingDouble;
  _pendingDouble = false;
  return ts;
}

/* Used to send debug information to the Serial Monitor
   TOF operates in Core 0 in parallel to the Arduino code
   running the Serial Monitor in Core 1. Using Serial.println( F("hi") )
   will often be clipped or ignored when run in Core 0. The main
   loop() in ReflectionsOS.ino calls TOF::getMef() and prints to
   Serial Monitor from there. */

String AccelSensor::getRecentMessage() {
  String myMefa = myMef;
  myMef = "";
  return myMefa;
}

String AccelSensor::getRecentMessage2() {
  String myMefa = myMef2;
  myMef2 = "";
  return myMefa;
}

void AccelSensor::setStatus(bool run) {
  runflag = run;
}

bool AccelSensor::getStatus() {
  return runflag;
}

 // Detect single and double clicks

void AccelSensor::handleClicks()
{
  unsigned long now = millis();

  if ( now - waittime < WAIT_TIME ) return;

  if ( firstpart && ( now - firstClickTime > 450 ) )
  {
    firstpart = false;
    _pendingSingle = true;
    waittime = now;
  }

  uint8_t click = lis.getClick();
  if (click == 0) return;
  if (! (click & 0x30)) return;

  if ( (click == 0x54 ) && ( now - minClickTime > 150 ) )
  {
    minClickTime = now;

    if ( !firstpart )
    {
      firstClickTime = now;
      firstpart = true;
      return;
    }

    if ( firstpart && ( now - firstClickTime > 450 ) )
    {
      _pendingSingle = true;
      firstpart = false;
      waittime = now;
    }

    if ( firstpart && ( now - firstClickTime < 450 ) )
    {
      _pendingDouble = true;
      firstpart = false;
      waittime = millis();
    }
  }
}

void AccelSensor::loop() 
{
  if ( ! runflag ) return;

  handleClicks();     // Detect single and double clicks

  if ( millis() - last < SAMPLE_RATE) return;
  last = millis();
  
  sensors_event_t event;
  lis.getEvent(&event);

  magnow = sqrt( ( event.acceleration.x * event.acceleration.x ) + ( event.acceleration.y * event.acceleration.y ) + ( event.acceleration.z * event.acceleration.z ) );
  
  // With enough movement, run the shaken experience
  if ( millis() - shakentime > 5000 )
  {
    shakentime = millis();
    shakencount = 0;
  }

  if ( ( magnow > 15 ) && ( millis() - shakentime2 > 200 ) )
  {
    shakentime2 = millis();
    shakencount++;

    /*
    Serial.print( "Accel shaken ");
    Serial.print( shakencount );
    Serial.print( " magnow " );
    Serial.println( magnow );
    */
  }

}  

