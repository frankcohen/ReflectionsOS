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
    while (1) yield();
  }

  // Set shake detection parameters
  lis.setRange(LIS3DH_RANGE_8_G);  // 2, 4, 8 or 16 G!

  lis.setDataRate(LIS3DH_DATARATE_400_HZ);  // Set ODR to 100 Hz

  fillValues();  // initializes SRAM-resident values[]

  // Compute threshold from the provided static array
  double median, mad;
  computeThreshold( aValues, AccelCalN, median, mad);
  threshold = median - mad;

  // Run the simulation test on aValues[N]
  runSimulation();

  state = 1;
  skipCount = 0;
  lookaheadCount = 0;
  sampleIndex = 0;
  last = millis();
  waittime = millis();

  started = true;
}

/*
Values used to calibrate the median and MAD for the sensor.
The sensor is noisy. I wasn't satisfied with the built-in
click detection. This helps detect single and double taps.

These values are me doing a double tap, single tap, double tap,
single tap, over 59 scans
*/

void AccelSensor::fillValues() {
  double temp[AccelCalN] = {
    12.1078280463508, 9.33203621938964, 10.1310512781251, 8.7236059058167,
    5.80487725968431, 9.57146801697629, 15.352641466536, 15.0080311833365,
    8.41240155960235, 9.1309473769155, 10.6754344173902, 8.17425225938128,
    8.91654641663464, 11.1541247975805, 10.8324974036461, 7.17878819857502,
    6.67956585415549, 14.1428321067599, 11.9598118714301, 10.1949840608017,
    11.6949647284633, 10.0960437796198, 9.97769512462673, 10.0360300916249,
    7.79964742792903, 7.6232407806654, 15.5585378490397, 12.153312305705,
    9.44136642652958, 10.1342685971904, 9.08789854696893, 9.51874466513311,
    8.6300579372331, 7.77512700860893, 10.8916527671424, 5.0360798246255,
    9.1321081903359, 5.78180767580521, 10.2642778606193, 12.3416084851206,
    10.8678102670225, 8.84833317636717, 9.31919524422576, 9.48536240741491,
    8.86901347388761, 9.50015263035284, 8.42765091825711, 8.16214432609471,
    6.78552135064064, 9.91885578078439, 9.55362234966403, 9.4088575289458,
    10.5058126767994, 11.1372303558829, 9.25156203027359, 10.5181129486234,
    10.103331133839, 9.87751993164276, 9.87634547795894
  };

  memcpy( aValues, temp, sizeof(temp));  // Copy into SRAM
}

// Compute median and MAD on an array of length N

void AccelSensor::computeThreshold(const double *vals, int n, double &outMedian, double &outMad) {
  static double sorted[AccelCalN];
  memcpy(sorted, vals, n * sizeof(double));
  for (int i = 1; i < n; ++i) {
    double key = sorted[i];
    int j = i - 1;
    while (j >= 0 && sorted[j] > key) {
      sorted[j + 1] = sorted[j];
      --j;
    }
    sorted[j + 1] = key;
  }
  // median
  outMedian = (n & 1)
                ? sorted[n / 2]
                : 0.5 * (sorted[n / 2 - 1] + sorted[n / 2]);
  // MAD
  double devs[AccelCalN];
  for (int i = 0; i < n; ++i) {
    devs[i] = fabs(vals[i] - outMedian);
  }
  for (int i = 1; i < n; ++i) {
    double key = devs[i];
    int j = i - 1;
    while (j >= 0 && devs[j] > key) {
      devs[j + 1] = devs[j];
      --j;
    }
    devs[j + 1] = key;
  }
  outMad = (n & 1)
             ? devs[n / 2]
             : 0.5 * (devs[n / 2 - 1] + devs[n / 2]);
}

void AccelSensor::runSimulation() {
  Serial.print(F("Threshold = median - MAD = "));
  Serial.println(threshold, 6);
  Serial.println(F("Starting tap simulation..."));

  state = 1;
  skipCount = 0;
  lookaheadCount = 0;
  firstIdx = -1;

  int i = 0;
  while (i < AccelCalN) {
    if (skipCount > 0) {
      --skipCount;
      ++i;
      continue;
    }
    double v = aValues[i];

    if (state == 1) {
      if (v < threshold) {
        // immediate next sample dip → SINGLE
        if (i + 1 < AccelCalN && aValues[i + 1] < threshold) {
          Serial.print(F("Single click at idx "));
          Serial.println(i);
          skipCount = SKIP_AFTER;
          ++i;
          continue;
        }
        state = 2;
        lookaheadCount = 0;
        firstIdx = i;
        firstMag = v;
      }
      ++i;
    } else {  // WAIT_SECOND
      ++lookaheadCount;
      if (v < threshold) 
      {
        _pendingDouble = true;
        _pendingSingle = false;
        Serial.print(F("Double click at idx "));
        Serial.println(i);
        state = 1;
        skipCount = SKIP_AFTER;
        ++i;
      } else if (lookaheadCount >= LOOKAHEAD_MAX) {
        Serial.print(F("Single click at idx "));
        Serial.println(firstIdx);
        _pendingSingle = true;
        _pendingDouble = false;
        state = 1;
        skipCount = SKIP_AFTER;
        ++i;
      } else {
        ++i;
      }
    }
  }

  Serial.println();
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

// Resets the sensor

void AccelSensor::resetLIS3DH() {
  // Read the current value of CTRL_REG5 (0x24)
  uint8_t ctrlReg5Value = 0;
  Wire.beginTransmission(0x18);  // Start communication with LIS3DH (I2C address)
  Wire.write(0x24);              // Register address for CTRL_REG5
  Wire.endTransmission(false);   // Keep the connection active
  Wire.requestFrom(0x18, 1);     // Request 1 byte of data
  if (Wire.available()) {
    ctrlReg5Value = Wire.read();  // Read the current value from CTRL_REG5
  }

  // Set the REBOOT_MEMORY bit (bit 7) to reset the device
  ctrlReg5Value |= 0x80;  // Set the most significant bit to 1 (0x80)

  // Write the new value back to CTRL_REG5 to reboot the sensor
  Wire.beginTransmission(0x18);  // Start communication again
  Wire.write(0x24);              // CTRL_REG5 register address
  Wire.write(ctrlReg5Value);     // Write the updated value
  Wire.endTransmission();        // End transmission

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

void AccelSensor::reset() {
  _pendingSingle = false;
  _pendingDouble = false;
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

void AccelSensor::loop() {
  unsigned long now = millis();

  if ( now - waittime < WAIT_TIME ) return;

  if (millis() - last < SAMPLE_RATE) return;
  last = now;

  ++sampleIndex;

  // Get live magnitude in m/s²
  sensors_event_t evt;
  lis.getEvent(&evt);
  double mag = sqrt(
    evt.acceleration.x * evt.acceleration.x + evt.acceleration.y * evt.acceleration.y + evt.acceleration.z * evt.acceleration.z);

  if (skipCount > 0) {
    --skipCount;
    return;
  }

  if (state == 1) {
    if (mag < threshold) 
    {
      state = 2;
      lookaheadCount = 0;
      firstIdx = sampleIndex;
      firstMag = mag;
      skipCount = 2;
      //Serial.println("*");
      return;
    }
  } 
  else 
  {  // WAIT_SECOND
    ++lookaheadCount;
    if (mag < threshold) 
    {
      _pendingDouble = true;
      _pendingSingle = false;

      String mef = String( sampleIndex );
      mef += mag;
      mef += ",";
      mef += threshold;
      Serial.print(F("Double click @ sample "));
      Serial.println( mef );

      state = 1;
      skipCount = SKIP_AFTER;
      waittime = now;
    }
    else if (lookaheadCount >= LOOKAHEAD_MAX) 
    {
      _pendingSingle = true;
      _pendingDouble = false;
      Serial.print(F("Single click @ sample "));
      Serial.println(firstIdx);
      state = 1;
      skipCount = SKIP_AFTER;
      waittime = now;
    }
  }
}
