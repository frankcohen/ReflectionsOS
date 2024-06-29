#ifndef _TOF_
#define _TOF_

#include "config.h"
#include "secrets.h"

#include <Wire.h>

#include <SparkFun_VL53L5CX_Library.h>

// Definitions for TOFeyes
#define tofdiam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define maxdist 50

class TOF
{
  public:
    TOF();
    void begin();
    void loop();
    bool test();
    void printTOF();
    bool tofStatus();
    int getNextGesture();
    int getXFingerPosition();
    bool cancelGestureDetected();

  private:
    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM
    int imageResolution = 0;  //Used to pretty print output
    int imageWidth = 0;       //Used to pretty print output
    bool started;

    SparkFun_VL53L5CX tofSensor;
    int detectionThreshold;
    int pollingInterval;
    int majorityThreshold;

    unsigned long lastPollTime;

    unsigned long cancelGestureTimeout;

    bool cancelDetected;

    void checkForCancelGesture();
    void removeExpiredGestures();

};

#endif // _TOF_
