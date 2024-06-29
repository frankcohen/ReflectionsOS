/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "TimeService.h"

extern LOGGER logger;
extern Arduino_GFX *gfx;
extern const char* root_ca;   // Defined in secrets.h
extern Battery battery;

TimeService::TimeService(){}

String TimeService::getRTCtime()
{
  struct tm timeinfo;
  if ( ! getLocalTime( &timeinfo ) ) 
  {
    return "0 o'clock";
  }

  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;
  String period = "AM";

  if (hour >= 12) {
    period = "PM";
    if (hour > 12) {
      hour -= 12;
    }
  } else if (hour == 0) {
    hour = 12; // Midnight case
  }

  String minuteStr = (minute < 10) ? "0" + String(minute) : String(minute);
  String timeStr = String(hour) + ":" + minuteStr + " " + period;

  return timeStr;
}

  // Clock fun messages
  
  const char* timetext[52][2] = {
  { "It's early",    "to be exact" },
  { "It's late",     "to be exact"},
  { "Wait, wait",    "you waited!"},
  { "Peekaboo",      "i see you"},
  { "Why?",          "why not?"},
  { "When?",         "and where?"},
  { "So little time","to be exact"},
  { "Will this end?","and when?"},
  { "Cats forever!", "meow"},
  { "Hug please",    "forever"},
  { "I'm late",      "important date"},
  {"We're all mad",  "don't worry"},
  {"Grin",           "like a cat."},
  {"Curiouser",      "and curiouser"},
  {"Most everyone",  "is mad here"},
  {"A dream",        "within a dream"},
  {"I've gone",      "entirely mad"},
  {"We're all mad",  "indeed"},
  {"Simply mad",     "as a hatter"},
  {"Twiddledee",     "and Twiddledum"},
  {"I vanish",       "like a ghost"},
  {"Very, very",     "mysterious"},
  {"Truly, very",    "wonderland"},
  {"I appear",       "and disappear"},
  {"Slithy toves",   "Brillig?"},
  {"Did gyre?",      "Borogroves?"},
  {"Do you play?",   "Croquet"},
  {"Follow",         "White rabbit"},
  {"See me?",        "Or not"},
  {"White rabbit",   "Who?"},
  {"I am mad",       "So are you"},
  {"Find me",        "if you can"},
  {"A place",        "like no other"},
  {"Do you?",        "Believe?"},
  {"Look closely",   "See nothing"},
  {"Smiling",        "Ear to ear"},
  {"Invisible",      "Yet here"},
  {"Peculiar",       "Strange"},
  };

void TimeService::begin()
{ 
  ShowTimeWaitTime = millis();
  activated = false;
  timeValid = false;
  theTime = "0 o'clock";
}

bool TimeService::test()
{
  return true;
}

boolean TimeService::fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, const GFXfont * font)
{
  if ( fadeset )
  {
    fadeset = 0;
    gfx->setFont( font );
    gfx->getTextBounds( text.c_str(), 0, 0, &x, &y, &w, &h);    

    fadestep = 1;
    steps = 128; // One step per color intensity value
    stepDelay = ( duration / steps ) / 4; // Delay between steps in milliseconds

    return false;
  }
  else
  {
    fadestep++;

    if ( fadestep >= steps ) return true;

    uint16_t r = map( fadestep, 0, steps, (COLOR_BLACK >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (COLOR_BLACK >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, COLOR_BLACK & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    gfx->setCursor( (gfx->width() - w) / 2, y );
    gfx->setTextColor( textColor );
    gfx->println( text );
  }

  return false;
}

boolean TimeService::fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, const GFXfont * font)
{
  if ( fadeset )
  {
    fadeset = 0;

    gfx->setFont( font );
    gfx->getTextBounds( text.c_str(), 0, 0, &x, &y, &w, &h);    

    fadestep = 128;
    steps = 128;
    stepDelay = ( duration / steps ) / 4;
    
    return false;
  }
  else
  {
    fadestep--;

    if ( fadestep == 0 ) return true;

    uint16_t r = map( fadestep, 0, steps, (COLOR_BLACK >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (COLOR_BLACK >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, COLOR_BLACK & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    gfx->setFont( font );
    gfx->setCursor( (gfx->width() - w) / 2, y );
    gfx->setTextColor( textColor );
    gfx->println( text );
  }
  return false;
}

void TimeService::startShow( int shownum )
{
  showNum = shownum;
  showStep = 0;
  activated = true;

  ShowTimeWaitTime = millis();
  stepDelay = 100;
}

// Function to get the number of pairs
int TimeService::getNumPairs() 
{
  return 0;
  
  //return sizeof( BatteryIcon ) / sizeof( BatteryIcon[0] );
}

void TimeService::runShowTellTime()
{
  if ( showStep == 0 )
  {
    gfx->invertDisplay(true);
    //gfx->fillScreen( COLOR_BACKGROUND );
    showStep = 1;
    fadeset = 1;
    theTime = getRTCtime();

    int index = random(0, 35);
    theMsg1 = timetext[ index ][ 0 ];
    theMsg2 = timetext[ index ][ 1 ];
    
    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theMsg1, 70, 100, COLOR_TEXT_YELLOW, &Some_Time_Later20pt7b ) )
    {
      showStep = 2;
      fadeset = 1;
    }
    else
    {
      return;
    }
  }

  if ( showStep == 2 )
  {
    if ( fadeInCenteredText( theTime, 105, 50, COLOR_STRIPE_MEDIUM_GRAY, &Minya16pt7b ) )
    {
      showStep = 3;
      fadeset = 1;
    }
    else
    {
      return;
    }
  }

  if ( showStep == 3 )
  {
   if ( fadeInCenteredText( theMsg2, 135, 10, COLOR_STRIPE_PINK, &ScienceFair14pt7b ) )
    {
      showStep = 4;
      fadeset = 1;
    }
    else
    {
      return;
    }
  }

  if ( showStep == 4 )
  {
   if ( fadeOutCenteredText( theMsg2, 135, 10, COLOR_STRIPE_PINK, &ScienceFair14pt7b ) )
    {
      showStep = 5;
      fadeset = 1;
    }
    else
    {
      return;
    }
  }

  if ( showStep == 5 )
  {
    if ( fadeOutCenteredText( theTime, 105, 50, COLOR_STRIPE_MEDIUM_GRAY, &Minya16pt7b ) )
    {
      showStep = 6;
      fadeset = 1;
    }
    else
    {
      return;
    }
  }

  if ( showStep == 6 )
  {
    if ( fadeOutCenteredText( theMsg1, 70, 100, COLOR_TEXT_YELLOW, &Some_Time_Later20pt7b ) )
    {
      activated = 0;
      showStep = 0;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }

}

bool TimeService::getActivated()
{
  return activated;
}

void TimeService::loop()
{

    if ( ( millis() - ShowTimeWaitTime ) > stepDelay )
    {
      ShowTimeWaitTime = millis();

      if ( ! activated )
      {
        // startShow( 0 );
        return;
      }

      // Show the time

      if ( showNum == 0 ) runShowTellTime();

    }
}
