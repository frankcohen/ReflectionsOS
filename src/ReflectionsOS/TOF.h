/*
  TOF Sensor Gesture Detection

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library

  Reflections board uses a Time Of Flight (TOF) VL53L5CX sensor to
  identify user gestures with their fingers and hand. Gestures control
  operating the Experiences.

  Datasheet comes with this source code, see: vl53l5cx-2886943_.pdf

  Note: 
  ST-supplied VL53L5CX API function vl53l5cx_get_ranging_data() allocates a very large 
  temporary buffer on the stack (enough to hold all 64 zones of data plus status and 
  signal info), and when you call getRangingData() rapidly in your TOF-loop task it 
  overruns the default FreeRTOS stack. While coding I found many stack crashes. In the
  backtrace—SysTick ISR running straight into vl53l5cx_get_ranging_data() — is a classic 
  symptom of a corrupted stack frame.
  To fix this I increased the Core0 task’s stack size from the default (~8 KB) to 16384
  in xTaskCreatePinnedToCore( in Reflections.ino.
  I also have these other possible fixes:
  - Drop from 8×8 to 4×4 zones.
  - Patch the library. vl53l5cx_api.cpp (around line 625 in v1.0.3) changes the local i2c
    read buffer from a stack allocation to a static or global:
    // before (stack allocation)
    uint8_t buf[VL53L5CX_RANGE_STREAM_COUNT * sizeof(VL53L5CX_RangingMeasurementData)];
    // after (static allocation)
    static uint8_t buf[VL53L5CX_RANGE_STREAM_COUNT * sizeof(VL53L5CX_RangingMeasurementData)];
    In this way it lives in .bss instead of on your task stack.
*/

#ifndef TOF_H
#define TOF_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>

// ——— CONFIGURATION ———
#define SDA_PIN               3
#define SCL_PIN               4
#define TOF_POWER_PIN        26

#define FRAME_RATE            8                   // Hz
#define FRAME_INTERVAL_MS    (1000/FRAME_RATE)    // ms between valid frames

#define MIN_DIST_MM       18    // ignore anything closer
#define MAX_DIST_MM      130    // ignore anything farther (no object)
#define VALID_PIXEL_COUNT 30    // min count of valid cells per frame

#define WINDOW             5    // number of frames to buffer
#define SWIPE_TIMEOUT_MS  500   // ms to ignore until next swipe

#define GESTURE_WAIT      2000  // Time between gestures
#define CIRCULAR_MAX      6    // Count up to circular detection

#define FRAME_RATE         8    // Hz
#define FRAME_INTERVAL_MS (1000/FRAME_RATE)

#define lrmesg F("← left-to-right detected ")
#define rlmesg F("→ right-to-left detected ")
#define cirmesg F("→← circular detected ")

// Sleep detection thresholds
#define SLEEP_MIN_DISTANCE    5                   // mm
#define SLEEP_MAX_DISTANCE   12                   // mm
#define SLEEP_COVERAGE_COUNT 43                   // # pixels in [0,15] → “covered”
#define SLEEP_HOLD_MS      1500UL                 // ms of coverage to declare Sleep

#define MAX_SCANTIME 1000                         // Maximum time to form a gesture

// Finger-tip detection thresholds
#define TIP_MIN_DISTANCE     15                   // mm
#define TIP_MAX_DISTANCE    130                   // mm

// Frame dimensions
#define WIDTH                 8
#define HEIGHT                8

// Gesture codes
#define GESTURE_NONE 0
#define GESTURE_LEFT_RIGHT 1
#define GESTURE_RIGHT_LEFT 2
#define GESTURE_CIRCULAR 3
#define GESTURE_SLEEP 4

class TOF {
public:
  TOF();

  // Initialize the VL53L5CX sensor and internal state
  void begin();

  // Call periodically from Arduino loop(); performs gesture detection
  void loop();

  // Returns the last detected gesture code (see GESTURE_ defs). 0 if none.
  int  getGesture();

  // Enable or disable gesture detection
  void setStatus(bool running);

  // Retrieve current running status
  bool getStatus();

  // Get the most recent finger-column position (0 if none)
  int getFingerPos();
  float getFingerDist();  

  String getRecentMessage();
  String getRecentMessage2();

private:
  SparkFun_VL53L5CX tof;

  void resetGesture();
  void prettyPrintAllRotated();
  int mapFloatTo0_7(float input, float inMin, float inMax);

  // circular buffer of the last WINDOW rotated frames
  int16_t rotatedFrames[WINDOW][64];
  float centroidFrames[WINDOW];

  // sliding‐window of deltas for direction analysis
  float   deltas[WINDOW];
  float   lastCentroid;
  int     wi;

  unsigned long lastRead, captTime, lastValid, sleepStart;
  int circCnt;
  float sumDelta;

  bool pendingDirection;
  bool isRunning;

  // Gesture storage
  int direction;         // GESTURE_ code of last detected event
  String directionWay;   // human‐readable label (e.g. "→  (Right)")

  // Finger tip column (1..7) or 0 if none
  int tipPos;
  float tipDist;
  float tipMin;
  float tipMax;

  String mymessage;
  String mymessage2;
};

#endif // TOF_H
