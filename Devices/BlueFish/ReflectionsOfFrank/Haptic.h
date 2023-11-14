#ifndef _HAPTIC_
#define _HAPTIC_

#include "config.h"
#include "secrets.h"

#include "Adafruit_DRV2605.h"

class Haptic
{
  public:
    Haptic();
    void begin();
    bool test();
    void loop();
    void playEffect( int effect );

  private:
    Adafruit_DRV2605 drv;
    bool started;
};

#endif // _HAPTIC_
