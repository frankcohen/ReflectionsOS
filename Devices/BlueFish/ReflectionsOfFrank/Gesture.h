#ifndef _GESTURE_
#define _GESTURE_

#include "config.h"
#include "secrets.h"

#include <Wire.h>

#include <SparkFun_VL53L5CX_Library.h>

class Gesture
{
  public:
    Gesture();
    void begin();
    void loop();
    boolean test();
    void printGesture();
    
  private:
    SparkFun_VL53L5CX myImager;
    VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM
    int imageResolution = 0;  //Used to pretty print output
    int imageWidth = 0;       //Used to pretty print output
    boolean started;
    



    
};

#endif // _GESTURE_
