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

#define MIN_DISTANCE         15                   // mm
#define MAX_DISTANCE        130                   // mm
#define VALID_PIXEL_COUNT    50                   // pixels needed to consider a frame “valid”

#define CIRCULAR_TIME       500                   // ms to wait after swipe before declaring “circular”

// Sleep detection thresholds
#define SLEEP_MIN_DISTANCE    3                   // mm
#define SLEEP_MAX_DISTANCE   15                   // mm
#define SLEEP_COVERAGE_COUNT 45                   // # pixels in [0,15] → “covered”
#define SLEEP_HOLD_MS      3000UL                 // ms of coverage to declare Sleep

// Finger-tip detection thresholds
#define TIP_MIN_DISTANCE     15                   // mm
#define TIP_MAX_DISTANCE    130                   // mm

// Frame dimensions
#define WIDTH                 8
#define HEIGHT                8

// Gesture codes
#define GESTURE_NONE 0
#define GESTURE_RIGHT 1
#define GESTURE_UP_RIGHT 2
#define GESTURE_UP 3
#define GESTURE_UP_LEFT 4
#define GESTURE_LEFT 5
#define GESTURE_DOWN_LEFT 6
#define GESTURE_DOWN 7
#define GESTURE_DOWN_RIGHT 8
#define GESTURE_CIRCULAR 10
#define GESTURE_SLEEP 11

class TOF {
public:
  TOF();

  // Initialize the VL53L5CX sensor and internal state
  void begin();

  // Call periodically from Arduino loop(); performs gesture detection
  void loop();

  // Returns the last detected gesture code (see GESTURE_ defs). 0 if none.
  int  getGesture();

  // Enable or disable gesture detection (loop() will exit immediately if status is false)
  void setStatus(bool running);

  // Retrieve current running status
  bool getStatus();

  // Get the most recent finger-column position (0 if none)
  int getFingerPos();
  float getFingerDist();  

  String getRecentMessage();
  String getRecentMessage2();

private:
  // Internal methods
  void resetGesture();
  void computeOpticalFlow(const float frameA[HEIGHT][WIDTH],
                          const float frameB[HEIGHT][WIDTH],
                          float &u, float &v);

  // Sensor object
  SparkFun_VL53L5CX tof;

  // Timing & state variables
  unsigned long lastRead;
  unsigned long captTime;
  unsigned long timeLimit;
  unsigned long pendingTimer;
  static unsigned long sleepStart;

  bool pendingDirection;
  bool prevValid;
  bool isRunning;

  // Frame buffers
  float frame1[HEIGHT][WIDTH];
  float frame2[HEIGHT][WIDTH];

  // Gesture storage
  int direction;         // GESTURE_ code of last detected event
  String directionWay;   // human‐readable label (e.g. "→  (Right)")

  // Finger tip column (1..7) or 0 if none
  int tipPos;
  float tipDist;  

  String mymessage;
  String mymessage2;
};

#endif // TOF_H
