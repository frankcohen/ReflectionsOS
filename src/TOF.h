#ifndef _TOF_
#define _TOF_

#include "config.h"
#include "secrets.h"

#include "Logger.h"

#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <Arduino_GFX_Library.h>

#define TOF_BUFFER_SIZE 20
#define TOF_BLOCK_SIZE 64

// Definitions for TOFeyes
#define tofdiam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define maxdist 50

// Definitions for Cancel gesture
#define cancelDuration 2000
#define detectionThresholdLow 4
#define detectionThresholdHigh 25
#define majorityThreshold 38

#define gestureframes 5

#define GRID_ROWS 8
#define GRID_COLS 8

// Structure to store a single measurement

struct GestureData {
    int x;
    int y;
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
    void printTOF();

    bool tofStatus();
    void showBubbles();

    int getNextGesture();
    bool cancelGestureDetected();
    int getReadingsCount();

  private:
    // Buffer to store measurements

    GestureData* gestureBuffer;
    int bufferIndex;
    int currentBlockID;

    void detectGestures();
    void getMostRecentValues(GestureData recentValues[], int count);
    void initGestureBuffer();

    enum Gestured { cancelled, none } recentGesture;

    unsigned long paceTime;
    unsigned long delayTime;

    int closeReadingsCount;

    int horizx;
    int horizy;
    int horizd;

    uint16_t horizontal_distances[GRID_ROWS][GRID_COLS];
    uint16_t vertical_distances[GRID_ROWS][GRID_COLS];
    
    TaskHandle_t sensorInitTaskHandle;
    SparkFun_VL53L5CX sensor;
    
    static void sensorInitTaskWrapper(void * parameter);
    void sensorInitTask();

    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM
    int imageResolution = 0;  //Used to pretty print output
    int imageWidth = 0;       //Used to pretty print output
    bool started;
        
    unsigned long lastPollTime;

    unsigned long cancelGestureTimeout;

    bool cancelDetected;
    unsigned long nextCanceltime;
    bool nextCancelflag;

    void checkForCancelGesture();
    void removeExpiredGestures();

    // Helper methods for gesture detection
    bool detectCancelGesture();
    bool detectCircularGesture();
    bool detectLinearGesture(int x, int y);
    bool detectVerticalGesture();
    bool detectBombDropGesture();
    bool detectFlyAwayGesture();
    bool detectFingerTip();

    // Gesture handlers
    void Cancel_Gesture();
    void Pounce_Gesture();
    void EyesFollow_Gesture(int position);
    void Parallax_Gesture();
    void Shaken_Gesture();
    void Chastise_Gesture();
    void BombDrop_Gesture();
    void FlyAway_Gesture();

    // Constants for buffer and accumulator space
    static const int BUFFER_SIZE = 100;
    static const int CAPTURE_RATE = 50;
    static const int ACCUMULATOR_SIZE = 50;
    static const int MAX_RADIUS = 25;

    // Gesture detection parameters
    static const int linearThreshold = 10;
    static const int minPointsForLine = 5;
    static const int centerThreshold = 5;
    static const int radiusRange = 10;
    static const int gestureCooldown = 3000;
    
    // Parameters for gesture detection
    unsigned long lastGestureTime;
    bool eyesFollowMode;
    bool bombDropDetected;
    bool flyAwayDetected;

};

#endif // _TOF_
