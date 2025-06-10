/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_Parallax_H
#define Experience_Parallax_H

#include <Arduino.h>

#include "Experience.h"
#include "Logger.h"
#include "Video.h"
#include "WatchFaceMain.h"
#include "AccelSensor.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern WatchFaceMain watchfacemain;   // Uses WatchFaceBase::drawImageFromFile to draw images to the display
extern AccelSensor accel;

#define parallaxname F("Parallax ")

class Experience_Parallax : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    bool tearflag     = false;
    bool vidflag      = false;
    unsigned long dur       = 0;
    unsigned long parallaxWaitTime = 0;
    unsigned long paralaxDuration  = 0;

    // motion‐based parallax state
    float prevX       = 0.0f;
    float prevY       = 0.0f;
    float motionX     = 0.0f;
    float motionY     = 0.0f;
    const float motionAlpha = 0.2f;        // EMA smoothing for deltas

    int currentFrame          = 5;         // start center
    unsigned long lastMotionTime = 0;
    static const unsigned long decayDelay = 150;  // ms between decay steps
};

#endif // Experience_Parallax_H
