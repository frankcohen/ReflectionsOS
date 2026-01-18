/*
  TOF Sensor Gesture Detection
  (header comment unchanged)
*/

#ifndef TOF_H
#define TOF_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>
#include "Video.h"

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

// Gesture cooldown (time-based) — start at 600 ms
#define GESTURE_WAIT      600   // ms cooldown after a gesture fires

#define CIRCULAR_MAX      6    // Count up to circular detection

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

  void begin();
  void loop();

  int  getGesture();

  void setStatus(bool running);
  bool getStatus();

  int   getFingerPos();
  float getFingerDist();

  String getRecentMessage();
  String getRecentMessage2();

  void resetGesture();

private:
  SparkFun_VL53L5CX tof;

  void prettyPrintAllRotated();
  int mapFloatTo0_7(float input, float inMin, float inMax);

  // circular buffer of the last WINDOW rotated frames
  int16_t rotatedFrames[WINDOW][64];
  float   centroidFrames[WINDOW];

  // sliding-window of deltas for direction analysis
  float deltas[WINDOW];
  float lastCentroid;
  int   wi;

  unsigned long lastRead, captTime, lastValid, sleepStart;
  int   circCnt;
  float sumDelta;

  bool pendingDirection;
  bool isRunning;

  // Gesture storage
  int    direction;
  String directionWay;

  // Finger tip column (0..7) or 0 if none (your current semantics)
  int   tipPos;
  float tipDist;
  float tipMin;
  float tipMax;

  unsigned long fingerLastSeen;
  bool          fingerPresent;

  // NEW: gesture gate (cooldown + require finger to leave view to re-arm)
  enum class GestureGateState : uint8_t {
    Armed = 0,
    Cooldown = 1
  };
  GestureGateState gateState;
  uint32_t         cooldownUntilMs;

  String mymessage;
  String mymessage2;
};

#endif // TOF_H