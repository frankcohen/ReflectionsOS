// Experience1.h
#ifndef Experience_Mystic_H
#define Experience_Mystic_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "Mystic.h"

class Experience_Mystic : public Experience {
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

#endif // Experience_Mystic_H
