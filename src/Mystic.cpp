#include "Mystic.h"
#include <Arduino_GFX_Library.h>

extern LOGGER logger;
extern Arduino_GFX *gfx;

Mystic::Mystic(){}

const char* affirmative[11][2] = {
{"Even being mad", "the answer, yes"},
{"Oh, indeed, yes", "silly you!"},
{"Absolutely, yes", "obviously"},
{"A time for change.", "Yes dear yes"},
{"Yes", "without question."},
{"Imagine reality", "yes most yes"},
{"My reality not the same", "still yes"},
{"It's certain", "what else?"},
{"Indubitably", "my dear!"},
{"Affirmative", "surprisingly."},
{"Figure it out, or not!", "Do it today"}
};


const char* noncommital[11][2] = {
{"Not crazy", "just don't see it yet"},
{"Adventures require", "a first step"},
{"Haste makes waste", "Try again"},
{"Not all who wander", "are last"},
{"I prefer ", "the short-cut"},
{"If you don't know", "neither do I"},
{"Proper order?",  "It's a mystery to me"},
{"I can't know", "everything"},
{"Clear as a", "clouded night"},
{"Yes... or no.", "Maybe both!"},
{"Where's the fun", "in knowing?"}
};

const char* negative[11][2] = {
{"Think you're getting away with it?", "Nope."},
{"Play fair?", "Not today"},
{"I'm not all there myself", "so no, no no"},
{"No time for joy", "nor play, no"},
{"Right on time", "just not today"},
{"Hidden in", "plain sight."},
{"We're all mad here", "dear."},
{"It's late", "to be exact"},
{"Wait, wait",    "you waited!"},
{"Absolutely, Not", "No"},
{"No", "without question."}
};


void Mystic::begin()
{ 
  ShowTimeWaitTime = millis();
  
}

bool Mystic::test()
{
  return true;
}

boolean Mystic::fadeInCenteredText( const char* text, int16_t x, int16_t y, uint16_t color, const GFXfont * font)
{
    gfx->fillScreen(BLACK);
    gfx->setTextSize(1);
    gfx->setFont(font);
    gfx->setTextColor(color);
    gfx->setCursor(x, y);
    gfx->print(text);
}

boolean Mystic::fadeOutCenteredText( const char* text, int16_t x, int16_t y, uint16_t color, const GFXfont * font)
{
    //gfx->fillScreen(BLACK);
    gfx->setTextSize(1);
    gfx->setFont(font);
    gfx->setTextColor(color);
    gfx->setCursor(x, y);
    gfx->print(text);
}

void Mystic::runShowTellAnswers()
{
  if ( showStep == 0 )
  {
    gfx->invertDisplay(true);
    //gfx->fillScreen( COLOR_BACKGROUND );
    
    int pIndex = random(0, 2);
    int sIndex = random(0, 10);
    
    if(pIndex == 0){
      theMsg1 = affirmative[ sIndex ][ 0 ];
      theMsg2 = affirmative[ sIndex ][ 1 ];
    }
    if(pIndex == 1){
      theMsg1 = noncommital[ sIndex ][ 0 ];
      theMsg2 = noncommital[ sIndex ][ 1 ];
    }
    if(pIndex == 2){
      theMsg1 = negative[ sIndex ][ 0 ];
      theMsg2 = negative[ sIndex ][ 1 ];
    }
  }

  fadeInCenteredText( theMsg1, 15, 100, COLOR_TEXT_YELLOW, &Some_Time_Later20pt7b);

  fadeOutCenteredText( theMsg2, 15, 125, COLOR_TEXT_YELLOW, &ScienceFair14pt7b);

  delay(10000);

  //fadeOutCenteredText( theMsg2, 15, 115, COLOR_TEXT_YELLOW, &ScienceFair14pt7b);

  gfx->fillScreen(BLACK);

}

void Mystic::loop()
{
  /*if ( ( millis() - ShowTimeWaitTime ) > stepDelay )
  {
    ShowTimeWaitTime = millis();
    if ( showNum == 0 ) runShowTellAnswers();
  }*/
}
