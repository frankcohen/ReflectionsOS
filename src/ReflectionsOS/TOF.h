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

#include <cmath>

extern Arduino_GFX *gfx;

#define GRID_ROWS 8
#define GRID_COLS 8
#define SET_SIZE 64      // Each block has these many readings
#define NUM_SETS 100     // Saves the most recent 100 blocks

#define FRAMES_TO_ANALYZE 10  // Use the most recent 10 frames for analysis

// Filter results to this range
#define closefilter 5
#define farfilter 60

// Definitions for Bubles
#define tofdiam 17
#define xdistance 26
#define ydistance 26
#define xspace 26
#define yspace 26
#define tofmaxdist 100

// Definitions for Sleep gesture
#define sleepLowFilter 5
#define sleepHighFilter 18
#define sleepPercentage 0.85

// Definitions for fingerTip dected gesture
#define fingerDetectionThresholdLow 18
#define fingerDetectionThresholdHigh 40

// Horizontal and vertical movement detection
#define movementLow 30
#define movementHigh 70
#define movementFrames 5

// Bubble range
#define bubbleLow 10
#define bubbleHigh 70

// Definitions for BombDrop and FlyAway gesture
#define bombFlyDistLow 25    // Cannot be smaller than fingerDetectionThresholdLow
#define bombFlyDistHigh 50   // Cannot be larger than fingerDetectionThresholdHigh

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
      FlyAway,
      Hover
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

    bool detectHorizontalGesture();
    bool detectHorizontalGesture2();
    bool detectHorizontalGesture3();
    bool detectHorizontalGesture4();
    bool detectVerticalGesture();

    bool detectCircularGesture();
    bool detectBombDropGesture();
    bool detectFlyAwayGesture();

    void flipAndRotateArray( int16_t* dest, int width, int height );

    bool started;
    TOFGesture recentGesture;

    SparkFun_VL53L5CX sensor;

    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

    int16_t* buffer;          // Wrap around buffer stores most recent measurements
    int currentSetIndex;

    int previousHorizPositions[ 8 ];
    int previousVertPositions[ 8 ];
    
    unsigned long gestureTime;
    
    int sleepCount;

    bool fingerTipInRange;
    int fingerPosRow;
    int fingerPosCol;
    float fingerDist;

    bool bombaccumulator[ 8 ];
    bool flyaccumulator[ 8 ];

    unsigned long lastPollTime;    
    unsigned long previousMillis;
    
    unsigned long debouncetime;

    void acquireDataToBuffer();

    String myMef;
    String myMef2;

    void determineMovementBetweenFrames( int16_t * olderFrame, int16_t * newerFrame, int &best_dx, int &best_dy);
    float computeCorrelation( int16_t * frame1, int16_t * frame2, int dx, int dy);


};

#endif // _TOF_
