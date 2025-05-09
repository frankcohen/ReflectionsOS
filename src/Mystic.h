#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

extern const char* affirmative[][2];
extern const char* noncommital[][2];
extern const char* negative[][2];

class Mystic
{
  public:
    Mystic();
    void begin();
    void loop();
    bool test();
    boolean fadeInCenteredText( const char* text, int16_t x, int16_t y, uint16_t color, const GFXfont * font);
    boolean fadeOutCenteredText( const char* text, int16_t x, int16_t y, uint16_t color, const GFXfont * font);
    void runShowTellAnswers();

  private:
    int16_t x, y;
    uint16_t w, h;
    int fadestep;
    uint16_t steps;
    uint16_t stepDelay;
    int showNum;
    long ShowTimeWaitTime;
    bool fadeset;
    int showStep;
    const char* theMsg1;
    const char* theMsg2;


};
