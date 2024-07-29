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

// Definitions for Bubles
#define tofdiam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define tofmaxdist 100

// Definitions for Cancel gesture
#define cancelDuration 2000
#define detectionThresholdLow 18
#define detectionThresholdHigh 30
#define cancelHighRejection 100
#define cancelRejectCount 2
#define minorityThreshold 15
#define majorityThreshold 19

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

    int closeReadingsCount;
    int maxCount;

    unsigned long paceTime;
    unsigned long delayTime;

    unsigned long fingerTime;
    bool fingerTipInRange;
    int fingerPosRow;
    int fingerPosCol;
    float fingerDist;

    float ssmin;
    float ssmax;
    float ssavg;
    float ssttl;
    float sscnt;

    unsigned long circularTimeOut;
    bool accumulator[ 4 ];

    unsigned long horizTimeOut;
    #define horizDetectionDuration 1500
    bool horizaccumulator[ 4 ];

    unsigned long vertTimeOut;
    #define vertDetectionDuration 1500
    bool vertaccumulator[ 4 ];

    unsigned long bombTimeOut;
    #define bombDetectionDuration 1500
    bool bombaccumulator[ 4 ];

    unsigned long flyTimeOut;
    #define flyDetectionDuration 1500
    bool flyaccumulator[ 4 ];

    unsigned long lastPollTime;    
};

#endif // _TOF_
