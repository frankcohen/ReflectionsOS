#ifndef _BATTERY_
#define _BATTERY_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

class Battery
{
  public:
    Battery();
    void begin();
    void loop();
    bool test();

  private:
    long batteryWaitTime;
};

#endif // _BATTERY_
