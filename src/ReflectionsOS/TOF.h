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

extern Arduino_GFX *gfx;

const int SET_SIZE = 64;      // Each block has these many readings
const int NUM_SETS = 100;     // Saves the most recent 100 blocks
#define gestureSensingDuration 1500   // Use most recent readings to sense a gesture

#define closefilter 15
#define farfilter 60

// Definitions for Bubles
#define tofdiam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define tofmaxdist 100

// Definitions for Sleep gesture
#define sleepLowFilter 10
#define sleepHighFilter 18
#define sleepPercentage 0.50

// Definitions for fingerTip dected gesture
#define fingerDetectionThresholdLow 15
#define fingerDetectionThresholdHigh 60

// Definitions for Circular gesture
#define circularDetectionDuration 2000

// Definitions for BombDrop and FlyAway gesture
#define bombFlyDistLow 25    // Cannot be smaller than fingerDetectionThresholdLow
#define bombFlyDistHigh 50   // Cannot be larger than fingerDetectionThresholdHigh

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
    String getStats();

    String getMef();
    String getMef2();

  private:
    void detectGestures();

    // Helper methods for gesture detection
    bool detectFingerTip( int setnum );
    bool detectSleepGesture();
    bool detectLeftToRight();

    bool detectCircularGesture();
    bool detectHorizontalGesture();
    bool detectVerticalGesture();
    bool detectBombDropGesture();
    bool detectFlyAwayGesture();


    bool started;
    TOFGesture recentGesture;

    SparkFun_VL53L5CX sensor;

    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

    int16_t* buffer;          // Wrap around buffer stores most recent measurements
    int currentSetIndex;

    unsigned long gestureTime;
    
    int sleepCount;

    bool fingerTipInRange;
    int fingerPosRow;
    int fingerPosCol;
    float fingerDist;

    bool accumulator[ 4 ];
    bool horizaccumulator[ 4 ];
    bool vertaccumulator[ 4 ];
    bool bombaccumulator[ 4 ];
    bool flyaccumulator[ 4 ];

    unsigned long lastPollTime;    

    unsigned long previousMillis;
    
    void acquireDataToBuffer();

    String myMef;
    String myMef2;

};

#endif // _TOF_
