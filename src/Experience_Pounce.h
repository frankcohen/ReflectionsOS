// Experience1.h
#ifndef Experience_Pounce_H
#define Experience_Pounce_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

class Experience_Pounce : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    
};

#endif // Experience_Pounce_H
