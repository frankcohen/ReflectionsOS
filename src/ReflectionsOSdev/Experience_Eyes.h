/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_Eyes_H
#define Experience_Eyes_H

#include <Arduino.h>
#include <math.h>

#include "Experience.h"
#include "TextMessageService.h"
#include "AccelSensor.h"

#include "Logger.h"
#include "Video.h"
#include "TOF.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TOF tof;
extern Arduino_GFX *gfx;
extern AccelSensor accel;

#define eyesname F("Eyes ")

class Experience_Eyes : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    unsigned long timer;
    bool timeflag;
    bool vidflag;
    unsigned long eyestime;
    unsigned long dur;

    unsigned long pace;

    int prevFingerPosCol;
    float prevFingerDist;

    int prevLeftPupilX;
    int prevRightPupilX;

    int eyeposx;
    int eyeposy;
    int eyedist;

    int  _eyes_finalCol = 5;     // current final column [1..9]
    bool _eyes_hasEma    = false;
    float _eyes_ema      = 5.0f; // EMA state on column domain 1..9

    // Hysteresis / stickiness to prevent flicker between TOF/Accel sources
    // Counts consecutive frames with/without TOF before switching source.
    uint8_t _eyes_stickyCount = 0;
    bool    _eyes_usingTOF    = false;

    // Tunables
    static constexpr float  EYES_EMA_ALPHA      = 0.45f; // 0..1 (higher = snappier)
    static constexpr uint8_t EYES_STICKY_FRAMES = 2;     // require N frames to flip source

    // Helpers
    int  eyesComputeAccelCol() const;                         // map tilt → [1..9]
    int eyesFuseColumns(int tofCol9);

    static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

#endif // Experience_Eyes_H
