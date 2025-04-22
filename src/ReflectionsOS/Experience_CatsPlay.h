/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_CatsPlay_H
#define Experience_CatsPlay_H

#include <Arduino.h>

#include "Experience.h"
#include "Haptic.h"
#include "BLEsupport.h"
#include "Compass.h"
#include "AccelSensor.h"
#include "Logger.h"
#include "Video.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;

extern TOF tof;
extern Compass compass;
extern Haptic haptic;
extern AccelSensor accel;
extern BLEsupport blesupport;

#define catsplayname F("Catsplay ")

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

    float lastHeading;               // stores last compass heading value
    unsigned long lastHeadingChangeTime; // timestamp when heading last changed
};

#endif // Experience_CatsPlay
