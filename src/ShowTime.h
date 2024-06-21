#ifndef _SHOWTIME_
#define _SHOWTIME_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "time.h"

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts

#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

extern const char* pairs[][2];

class ShowTime
{
  public:
    ShowTime();
    void begin();
    void loop();
    bool test();

    boolean fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, const GFXfont * font);
    boolean fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, const GFXfont * font);
    void startShow( int shownum );

    String getRTCtime();

  private:

    int16_t x, y;
    uint16_t w, h;
    int fadestep;
    uint16_t steps;
    uint16_t stepDelay;
    int showNum;
    bool activated;
    long ShowTimeWaitTime;
    bool fadeset;
    int showStep;
    String theMsg1;
    String theMsg2;

    void runShowTellTime();

    int getNumPairs();
    void drawSpriteOverBackground( const uint16_t *sprite, int16_t spriteWidth, int16_t spriteHeight, int16_t x, int16_t y, uint16_t transparent );

    bool timeValid;
    struct tm timeinfo;
    String theTime;
};

#endif // _SHOWTIME_
