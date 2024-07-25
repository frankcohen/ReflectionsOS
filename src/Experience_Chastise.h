// Experience1.h
#ifndef Experience_Chastise_H
#define Experience_Chastise_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

class Experience_Chastise : public Experience {
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

#endif // Experience_Chastise_H
