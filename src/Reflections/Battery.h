#ifndef _BATTERY_
#define _BATTERY_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Video.h"

#define batterylow 3900
#define battermedium 4000
#define batteryhigh 4000

class Battery
{
  public:
    Battery();
    void begin();
    void loop();
    bool test();
    String batLevel( float analogVolts );
    bool isBatteryLow();
    void drawLowBatteryIcon();
    int getBatteryLevel();

  private:
    long batteryWaitTime;
    float analogVolts;
    
    void drawSpriteOverBackground( const uint16_t *sprite, int16_t spriteWidth, int16_t spriteHeight, int16_t x, int16_t y, uint16_t transparent );
};

#endif // _BATTERY_
