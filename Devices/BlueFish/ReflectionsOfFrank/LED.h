#ifndef _LED_
#define _LED_

#include "config.h"
#include "secrets.h"

// #define FASTLED_INTERNAL
// #include <FastLED.h>

#include "Arduino.h"

class LED
{
  public:
    LED();
    void begin();
    void loop();

    void show();
    void setBlue( int lednum );
    void setAllBlack();

  private:
    //CRGB leds[ LED_Count ];
};

#endif // _LED_
