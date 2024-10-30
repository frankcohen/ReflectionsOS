/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _TOF_
#define _TOF_

#include "Arduino.h"

#include "config.h"
#include "secrets.h"
#include "Logger.h"

#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <Arduino_GFX_Library.h>
#include <cmath> // for abs function

const int SET_SIZE = 64;      // Each block has these many readings
const int NUM_SETS = 100;     // Saves the most recent 100 blocks

// Gesture timing
#define gestureSensingDuration 1500   // Take 1.5 seconds to sense a gesture, then try again

// Definitions for Bubles
#define tofdiam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define tofmaxdist 100

// Definitions for Sleep gesture
#define sleepHighRejection 26
#define sleepRejectCount 5
#define minorityThreshold 12
#define majorityThreshold 18
#define sleepDuration 4000
#define sleepRepeat 6

// Definitions for fingerTip dected gesture
#define fingerDetectionThresholdLow 28
#define fingerDetectionThresholdHigh 50
#define fingerWiggleRoom 7

// Definitions for Circular gesture
#define circularDetectionDuration 2000

// Definitions for BombDrop and FlyAway gesture
#define bombFlyDistLow 25    // Cannot be smaller than fingerDetectionThresholdLow
#define bombFlyDistHigh 50   // Cannot be larger than fingerDetectionThresholdHigh

#define gestureframes 5

#define GRID_ROWS 8
#define GRID_COLS 8

class TOF
{
  public:
    TOF();

    enum TOFGesture
    {
      None,
      Sleep,
      Circular,
      Horizontal,
      Vertical,
      BombDrop,
      FlyAway
    };

    void begin();
    void loop();
    bool test();

    bool tofStatus();

    TOFGesture getGesture();

    void displayStatus();
    void printTOF();
    void showBubbles();

  private:
    void detectGestures();

    // Helper methods for gesture detection
    bool detectSleepGesture();
    bool detectCircularGesture();
    bool detectFingerTip();
    bool detectHorizontalGesture();
    bool detectVerticalGesture();
    bool detectBombDropGesture();
    bool detectFlyAwayGesture();

    bool started;
    TOFGesture recentGesture;

    SparkFun_VL53L5CX sensor;

    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

    int16_t* buffer;          // Wrap around buffer stores most recent measurements
    int currentSetIndex = 0;

    unsigned long gestureTime;

    int closeReadingsCount;
    int maxCount;

    bool fingerTipInRange;
    int fingerPosRow;
    int fingerPosCol;
    float fingerDist;

    int sleepCount;
    unsigned long sleepTimer;
    float ssmin;
    float ssmax;
    float ssavg;
    float ssttl;
    float sscnt;

    bool accumulator[ 4 ];
    bool horizaccumulator[ 4 ];
    bool vertaccumulator[ 4 ];
    bool bombaccumulator[ 4 ];
    bool flyaccumulator[ 4 ];

    unsigned long lastPollTime;    
};

#endif // _TOF_
