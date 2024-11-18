/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_ShowTime_H
#define Experience_ShowTime_H

#include <Arduino.h>

#include "Experience.h"
#include "TextMessageService.h"

#include "Logger.h"
#include "Video.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TextMessageService textmessageservice;

class Experience_ShowTime : public Experience {
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

#endif // Experience_ShowTime_H
