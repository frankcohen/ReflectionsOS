// Experience1.h
#ifndef Experience_Shaken_H
#define Experience_Shaken_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"

class Experience_Shaken : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    
};

#endif // Experience_Shaken
