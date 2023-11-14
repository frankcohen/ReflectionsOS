#ifndef _ACCELLEROMETER_
#define _ACCELLEROMETER_

#include "config.h"
#include "secrets.h"

#include "LIS3DHTR.h"
#include <Wire.h>

class Accellerometer
{
  public:
    Accellerometer();
    void begin();
    void loop();
    boolean test();
    void printValues();
    
  private:
    LIS3DHTR<TwoWire> LIS;  
};

#endif // _ACCELLEROMETER_
