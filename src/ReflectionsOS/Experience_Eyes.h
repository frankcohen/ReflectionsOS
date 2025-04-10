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

#include "Experience.h"
#include "TextMessageService.h"

#include "Logger.h"
#include "Video.h"
#include "TOF.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TOF tof;
extern Arduino_GFX *gfx;

#define eyesname F("Eyes ")

class Experience_Eyes : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    unsigned long timer;
    bool tearflag;
    bool timeflag;
    bool vidflag;
    unsigned long eyestime;
    unsigned long dur;

    int prevFingerPosRow;
    int prevFingerPosCol;
    float prevFingerDist;

    int prevLeftPupilX;
    int prevRightPupilX;

    int eyeposx;
    int eyeposy;
    int eyedist;
    
};

#endif // Experience_Eyes_H
