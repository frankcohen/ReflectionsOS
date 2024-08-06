// Experience1.h
#ifndef Experience_CatsPlay_H
#define Experience_CatsPlay_H

#include <Arduino.h>

#include "Experience.h"

#include "BLE.h"
#include "Compass.h"

#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

class Experience_CatsPlay : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    bool setupVidplayed;
    unsigned long directionTimer;
    unsigned long overallTimer;
    float calculateBearing( float headingA, float headingB, float rssi );

};

#endif // Experience_CatsPlay
