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

#define accxleft 0.62
#define accxright 1.54
#define accpositions 5
#define stepvalue 0.184

class Experience_Parallax : public Experience {
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

    int pictureNum;
    unsigned long parallaxWaitTime;
    unsigned long paralaxDuration;
};

#endif // Experience_Parallax_H
