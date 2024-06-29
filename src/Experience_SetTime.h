// Experience1.h
#ifndef Experience_SetTime_H
#define Experience_SetTime_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

class Experience_SetTime : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;

  private:
    unsigned long timer;
    bool tearflag;
    bool timeflag;
    
};

#endif // Experience_SetTime_H
