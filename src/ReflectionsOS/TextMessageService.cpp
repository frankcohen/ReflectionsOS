/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "TextMessageService.h"

TextMessageService::TextMessageService(){}

// Clock fun messages

const char* timetext[51][2] = {
  { "It's early",    "to be exact" },
  { "It's late",     "to be exact"},
  { "Wait, wait",    "you waited!"},
  { "Peekaboo",      "i see you"},
  { "Why?",          "why not?"},
  { "When?",         "and where?"},
  { "Little time",   "to be exact"},
  { "Will it end?",  "and when?"},
  { "Cats forever",  "meow"},
  { "Hug please",    "forever"},
  { "I'm late",      "important date"},
  { "No panic",      "no worries"},
  { "Grin",          "like a cat"},
  { "Curiouser",     "and curiouser"},
  { "A dream",       "within a dream"},
  { "I've gone",     "entirely mad"},
  { "All mad",       "all of us"},
  { "Simply mad",    "as a hatter"},
  { "Twiddledee",    "and Twiddledum"},
  { "I vanish",      "like a ghost"},
  { "Very, very",    "mysterious"},
  { "Truly, very",   "wonderland"},
  { "I appear",      "and disappear"},
  { "Slithy toves",  "Brillig?"},
  { "Did gyre?",     "Borogroves?"},
  { "Play time?",    "Croquet"},
  { "Follow",        "White rabbit"},
  { "See me?",       "Or not"},
  { "A rabbit?",     "White even?"},
  { "I am mad",      "So are you"},
  { "Find me",       "if you can"},
  { "A place",       "like no other"},
  { "Do you?",       "Believe?"},
  { "Look closely",  "See nothing"},
  { "Smiling",       "Ear to ear"},
  { "Invisible!",    "Yet here"},
};

void TextMessageService::begin()
{ 
  ShowTimeWaitTime = millis();
  activated = false;
  timeValid = false;
  theTime = "0 o'clock";
  timeShowing = false;
  dialActivated = false;
  stTime = millis();

  stepDelay = 100;

  carrotAngle = 0.0;
  lastMoveTime = millis();
  moving = false;
  prior = false;
  String pastTimeStr = " ";
}

void TextMessageService::startShow( int shownum )
{
  showNum = shownum;
  showStep = 0;
  activated = true;
  ShowTimeWaitTime = millis();
  stepDelay = 100;
}

bool TextMessageService::isTimeSet()
{
  if ( theTime == "0 o'clock" ) return false;
  return true;
}

/* Show the current time with a funny message above and below */

void TextMessageService::runShowTellTime()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    //theTime = getRTCtime();       // This introduces a 3-5 second delay

    theTime = "2:43 o'clock";
    //if ( theTime == "0 o'clock" ) theTime = " ";

    int index = random(0, 37);
    theMsg1 = timetext[ index ][ 0 ];
    theMsg2 = timetext[ index ][ 1 ];
    
    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theMsg1, 90, 100, COLOR_TEXT_YELLOW, COLOR_BLACK, &Some_Time_Later20pt7b ) )
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
    if ( fadeInCenteredText( theTime, 130, 50, COLOR_STRIPE_MEDIUM_GRAY, COLOR_BLACK, &Minya16pt7b ) )
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
   if ( fadeInCenteredText( theMsg2, 170, 10, COLOR_STRIPE_PINK, COLOR_BLACK, &ScienceFair14pt7b ) )
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
   if ( fadeOutCenteredText( theMsg2, 170, 10, COLOR_STRIPE_PINK, COLOR_BLACK, &ScienceFair14pt7b ) )
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
    if ( fadeOutCenteredText( theTime, 130, 50, COLOR_STRIPE_MEDIUM_GRAY, COLOR_BLACK, &Minya16pt7b ) )
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
    if ( fadeOutCenteredText( theMsg1, 90, 100, COLOR_TEXT_YELLOW, COLOR_BLACK, &Some_Time_Later20pt7b ) )
    {
      activated = false;
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

// Fade-in digital time for Main Watch Face

void TextMessageService::runDigitalTimeFadeIn()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;

    theTime = getRTCtime();       // This introduces a 3-5 second delay    

    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theTime, 130, 15, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya_Nouvelle_Rg30pt7b ) )
    {
      activated = false;
      showStep = 2;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }
}

void TextMessageService::runDigitalTimeFadeOut()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;

    theTime = getRTCtime();       // This introduces a 3-5 second delay

    //if ( theTime == "0 o'clock" ) theTime = " ";
    
    theDate = "Nov 1, 2024";

    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeOutCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya_Nouvelle_Rg30pt7b ) )
    {
      activated = false;
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

// Shows digital time and a funny message

void TextMessageService::runTimeAndMessage()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    //theTime = getRTCtime();       // This introduces a 3-5 second delay

    theTime = "2:43 pm";
    //if ( theTime == "0 o'clock" ) theTime = " ";
    
    theDate = "Nov 1, 2024";

    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
      showStep = 2;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }

  if ( showStep == 2 )
  {
    if ( fadeOutCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
      showStep = 0;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }



/*
  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theTime, 130, 100, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
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
    if ( fadeInCenteredText( theDate, 170, 50, COLOR_STRIPE_MEDIUM_GRAY, COLOR_MAIN_BACK, &ScienceFair14pt7b ) )
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
    if ( fadeOutCenteredText( theDate, 170, 20, COLOR_STRIPE_MEDIUM_GRAY, COLOR_MAIN_BACK, &ScienceFair14pt7b ) )
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
    if ( fadeOutCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
      showStep = 0;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }

  */
}

// Draws centered text message

void TextMessageService::drawCenteredMesssage( String msg, String msg2 )
{
  bufferCanvas->setFont( &ScienceFair14pt7b );
  y = 115;
  bufferCanvas->getTextBounds( msg.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 115 );
  bufferCanvas->setTextColor( COLOR_TEXT_YELLOW );
  bufferCanvas->println( msg );

  y = 140;
  bufferCanvas->getTextBounds( msg2.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 140 );
  bufferCanvas->setTextColor( COLOR_TEXT_YELLOW );
  bufferCanvas->println( msg2 );
  bufferCanvas->flush();
}

// Redraws current time for wfMain

void TextMessageService::updateTime()
{
  theTime = getRTCtime();
  bufferCanvas->setFont( &Minya_Nouvelle_Rg30pt7b );
  y = 135;
  bufferCanvas->getTextBounds( theTime.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 127 );
  bufferCanvas->setTextColor( COLOR_TEXT_YELLOW );
  bufferCanvas->println( theTime );
  bufferCanvas->flush();
}

// Draws current time for wfMain set time panel

void TextMessageService::updateTempTime( String tempTime )
{
  bufferCanvas->setFont( &Minya_Nouvelle_Rg30pt7b );
  y = 135;
  bufferCanvas->getTextBounds( tempTime.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 135 );
  bufferCanvas->setTextColor( COLOR_TEXT_YELLOW );
  bufferCanvas->println( tempTime );
  bufferCanvas->flush();
}

String TextMessageService::formatWithCommas(int value)
{
  String formatted = "";
  String valStr = String(value);
  int length = valStr.length();

  int commaCount = (length - 1) / 3; // Calculate number of commas
  int pos = length;

  // Add commas at appropriate places
  while (commaCount > 0) {
      pos -= 3;
      formatted = "," + valStr.substring(pos, pos + 3) + formatted;
      commaCount--;
  }

  // Append the first part (without comma)
  formatted = valStr.substring(0, pos) + formatted;

  return formatted;
}

// Draws current time for wfMain Health panel

void TextMessageService::updateHealth( int smallsteps )
{
  bufferCanvas->setFont( &Some_Time_Later20pt7b );
  y = 100;
  String mef = "Steps";
  bufferCanvas->getTextBounds( mef.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 100 );
  bufferCanvas->setTextColor( COLOR_STRIPE_PINK );
  bufferCanvas->println( mef );

  bufferCanvas->setFont( &Minya16pt7b );
  y = 135;
  mef = formatWithCommas( smallsteps );
  bufferCanvas->getTextBounds( mef.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 135 );
  bufferCanvas->setTextColor( COLOR_TEXT_YELLOW );
  bufferCanvas->println( mef );

  bufferCanvas->flush();
}

// Draws current time for wfMain Timer

void TextMessageService::updateTimer( int minutesleft )
{
  bufferCanvas->setFont( &Minya_Nouvelle_Rg30pt7b );
  y = 135;
  String mef = (String) minutesleft;
  bufferCanvas->getTextBounds( mef.c_str(), 0, 0, &x, &y, &w, &h);
  bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, 135 );
  bufferCanvas->setTextColor( COLOR_TEXT_YELLOW );
  bufferCanvas->println( minutesleft );

  bufferCanvas->flush();
}

// Shows digital time for set time service

void TextMessageService::runDigitalTime()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    //theTime = getRTCtime();       // This introduces a 3-5 second delay

    theTime = "2:43 pm";
    //if ( theTime == "0 o'clock" ) theTime = " ";
    
    theDate = "Nov 1, 2024";

    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
      showStep = 2;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }

  if ( showStep == 2 )
  {
    if ( fadeOutCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
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

// Shows digital time for set time service

void TextMessageService::runDigitalSetTime()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    //theTime = getRTCtime();       // This introduces a 3-5 second delay

    theTime = "2:43 pm";
    //if ( theTime == "0 o'clock" ) theTime = " ";
    
    theDate = "Nov 1, 2024";

    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
      showStep = 2;
      fadeset = 1;
      ShowTimeWaitTime = millis();
    }
    else
    {
      return;
    }
  }

  if ( showStep == 2 )
  {
    if ( fadeOutCenteredText( theTime, 130, 30, COLOR_TEXT_YELLOW, COLOR_MAIN_BACK, &Minya16pt7b ) )
    {
      activated = false;
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

void TextMessageService::start()
{
  activated = true;
}

void TextMessageService::stop()
{
  activated = false;
}

bool TextMessageService::active()
{
  return activated;
}

String TextMessageService::getRTCtime()
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
  String timeStr = String(hour) + ":" + minuteStr;

  return timeStr;
}

boolean TextMessageService::fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font)
{
  if ( fadeset )
  {
    fadeset = 0;
    bufferCanvas->setFont( font );
    bufferCanvas->getTextBounds( text.c_str(), 0, 0, &x, &y, &w, &h);    

    fadestep = 1;
    steps = 128; // One step per color intensity value
    stepDelay = ( duration / steps ) / 4; // Delay between steps in milliseconds
    return false;
  }
  else
  {
    fadestep++;

    if ( fadestep >= steps ) return true;

    uint16_t r = map( fadestep, 0, steps, (backcolor >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (backcolor >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, backcolor & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, y );
    bufferCanvas->setTextColor( textColor );
    bufferCanvas->println( text );
    bufferCanvas->flush();
  }

  return false;
}

boolean TextMessageService::fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font)
{
  if ( fadeset )
  {
    fadeset = 0;

    bufferCanvas->setFont( font );
    bufferCanvas->getTextBounds( text.c_str(), 0, 0, &x, &y, &w, &h);    

    fadestep = 128;
    steps = 128;
    stepDelay = ( duration / steps ) / 4;
    
    return false;
  }
  else
  {
    fadestep--;

    if ( fadestep == 0 ) return true;

    uint16_t r = map( fadestep, 0, steps, (backcolor >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (backcolor >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, backcolor & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    bufferCanvas->setFont( font );
    bufferCanvas->setCursor( (bufferCanvas->width() - w) / 2, y );
    bufferCanvas->setTextColor( textColor );
    bufferCanvas->println( text );
    bufferCanvas->flush();
  }
  return false;
}

// TextMessageService loop

void TextMessageService::loop()
{
  // Run the Show Time text animation

  if ( activated )
  {    
    if ( ( millis() - ShowTimeWaitTime ) > stepDelay )
    {
      ShowTimeWaitTime = millis();

      switch ( showNum ) 
      {
        case ShowTellTime:
          runShowTellTime();
          break;
        case DigitalSetTime:
          runDigitalSetTime();
          break;
        case DigitalTimeFadeIn:
          runDigitalTimeFadeIn();
          break;
        case DigitalTimeFadeOut:
          runDigitalTimeFadeOut();
          break;
      }
    }
  }    

}
