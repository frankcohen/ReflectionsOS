/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_Sleep_H
#define Experience_Sleep_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "Hardware.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern Hardware hardware;

#define SleepName F("Sleep ")

class Experience_Sleep : public Experience {
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
    
};

#endif // Experience_Sleep_H
