#ifndef _TOF_
#define _TOF_

#include "config.h"
#include "secrets.h"

#include "Logger.h"

#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <Arduino_GFX_Library.h>
#include <cmath> // for abs function

#define BUFFER_SIZE 100
#define CAPTURE_RATE 50

#define TOF_BUFFER_SIZE 20
#define TOF_BLOCK_SIZE 64

// Definitions for TOFeyes
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
#define cancelRejectCount 5
#define majorityThreshold 50

// Definitions for fingerTip dected gesture
#define fingerDetectionThresholdLow 25
#define fingerDetectionThresholdHigh 50
#define fingerWiggleRoom 7

// Definitions for Circular gesture
#define circularDetectionDuration 1500

// Definitions for BombDrop and FlyAway gesture
#define bombFlyDistLow 25    // Cannot be smaller than fingerDetectionThresholdLow
#define bombFlyDistHigh 50   // Cannot be larger than fingerDetectionThresholdHigh

#define gestureframes 5

#define GRID_ROWS 8
#define GRID_COLS 8

// Structure to store a single measurement

struct GestureData {
    int distance;
    int blockID;
};

class TOF
{
  public:
    TOF();
    void begin();
    void loop();
    bool test();

    static void sensorInitTaskWrapper(void *parameter);
    void sensorInitTask();

    bool tofStatus();

    int getGesture();

    void printTOF();
    void showBubbles();

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

  private:
    TaskHandle_t sensorInitTaskHandle;

    void detectGestures();
    void getMostRecentValues(GestureData recentValues[], int count);
    void initGestureBuffer();

    // Helper methods for gesture detection
    bool detectSleepGesture();
    bool detectCircularGesture();
    bool detectFingerTip();
    bool detectHorizontalGesture();
    bool detectVerticalGesture();
    bool detectBombDropGesture();
    bool detectFlyAwayGesture();

    int recentGesture;

    bool fingerTipInRange;
    int fingerPosRow;
    int fingerPosCol;
    float fingerDist;

    SparkFun_VL53L5CX sensor;

    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM
    int imageResolution = 0;      //Used to pretty print output
    int imageWidth = 0;           //Used to pretty print output
    bool started;

    GestureData* gestureBuffer;       // Buffer to store most recent measurements
    int bufferIndex;
    int currentBlockID;

    unsigned long paceTime;
    unsigned long delayTime;

    unsigned long fingerTime;

    int closeReadingsCount;
    int maxCount;

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
