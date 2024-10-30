#ifndef _HARDWARE_
#define _HARDWARE_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "SD.h"
#include "SPI.h"
#include "Wire.h"
#include "Logger.h"

class Hardware
{
  public:
    Hardware();
    void begin();
    void loop();
    bool getMounted();

  private:
    bool NANDMounted;

};

#endif // _HARDWARE_
