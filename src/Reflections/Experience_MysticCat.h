// Experience1.h
#ifndef Experience_MysticCat_H
#define Experience_MysticCat_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

class Experience_MysticCat : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    
};

#endif // Experience_MysticCat
