/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Runs text shows as overlay on whatever is on the display at the moment

*/

#include "TextMessageService.h"

TextMessageService::TextMessageService(){}

void TextMessageService::begin()
{ 
  waitTime = millis();
  activated = false;
  theTime = F("0 o'clock");

  stepDelay = 100;
}

/*
  Displays msytic cat messages
*/

void TextMessageService::runMysticShow()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theMsg1, 80, 10, COLOR_TEXT_YELLOW, COLOR_BLACK, &Some_Time_Later12pt7b ) )
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
   if ( fadeInCenteredText( theMsg2, 110, 10, COLOR_STRIPE_PINK, COLOR_BLACK, &ScienceFair14pt7b ) )
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
   if ( fadeOutCenteredText( theMsg2, 110, 10, COLOR_STRIPE_PINK, COLOR_BLACK, &ScienceFair14pt7b ) )
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
    if ( fadeOutCenteredText( theMsg1, 80, 100, COLOR_TEXT_YELLOW, COLOR_BLACK, &Some_Time_Later12pt7b ) )
    {
      deactivate();
    }
    else
    {
      return;
    }
  }
}

void TextMessageService::startShow( int shownum, String msg1, String msg2 )
{ 
  showNum = shownum;
  showStep = 0;
  activated = true;
  waitTime = millis();
  stepDelay = 100;
  theMsg1 = msg1;
  theMsg2 = msg2;
}

void TextMessageService::deactivate()
{
  showNum = ShowDigitalTimeFunMessages;
  showStep = 0;
  activated = false;
  waitTime = millis();
  stepDelay = 100;
  fadeset = 1;
  theMsg1 = " ";
  theMsg2 = " ";
}

/* Show the current time with a funny message above and below */

void TextMessageService::runShowDigitalTimeFunMessages()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    theTime = realtimeclock.getTime();    
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
      deactivate();
    }
    else
    {
      return;
    }
  }

}

/* Watch face main digital time display */

void TextMessageService::runDigitalTime()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;
    theTime = realtimeclock.getTime();
    return;
  }

  if ( showStep == 1 )
  {
    if ( fadeInCenteredText( theTime, 127, 20, COLOR_TEXT_YELLOW, wfMain_BackgroundBlue, &Minya_Nouvelle_Rg30pt7b ) )
    {
      deactivate();
    }
  }
}

// Draws centered text message

void TextMessageService::drawCenteredMesssage( String msg, String msg2 )
{
  gfx->setFont( &ScienceFair14pt7b );
  gfx->setTextSize(0);
  y = 115;
  gfx->getTextBounds( msg.c_str(), 0, 0, &x, &y, &w, &h);
  gfx->setCursor( (gfx->width() - w) / 2, 115 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( msg );

  y = 140;
  gfx->getTextBounds( msg2.c_str(), 0, 0, &x, &y, &w, &h);
  gfx->setCursor( (gfx->width() - w) / 2, 140 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( msg2 );
}

// Redraws current time for wfMain

void TextMessageService::updateTime()
{
  theTime = realtimeclock.getTime();
  gfx->setFont( &Minya_Nouvelle_Rg30pt7b );
  y = 135;
  gfx->getTextBounds( theTime.c_str(), 0, 0, &x, &y, &w, &h);

  gfx->fillRect( x, y, w, h, wfMain_BackgroundBlue); // Clear to background

  gfx->setCursor( (gfx->width() - w) / 2, 127 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( theTime );
}

// Draws current time for wfMain set time panel

void TextMessageService::updateTempTime( String tempTime )
{
  gfx->setFont( &Minya_Nouvelle_Rg30pt7b );
  gfx->setTextSize( 0 );

  y = 135;

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;

  gfx->getTextBounds( tempTime.c_str(), 0, 0, &x1, &y1, &w, &h);
  gfx->fillRect( (gfx->width() - w) / 2, 106, w, h, wfMain_BackgroundBlue ); // Clear to background
  gfx->setCursor( (gfx->width() - w) / 2, 135 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( tempTime );
}

String TextMessageService::formatWithCommas(int value)
{
  String formatted = F("");
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
  gfx->setFont( &Some_Time_Later20pt7b );
  gfx->setTextSize( 0 );
  y = 100;
  String mef = F(" ");
  gfx->getTextBounds( mef.c_str(), 0, 0, &x, &y, &w, &h);
  gfx->setCursor( (gfx->width() - w) / 2, 100 );
  gfx->setTextColor( COLOR_STRIPE_PINK );
  gfx->println( mef );

  gfx->setFont( &Minya_Nouvelle_Rg30pt7b );
  gfx->setTextSize( 0 );

  y = 135;
  mef = formatWithCommas( smallsteps );
  gfx->getTextBounds( mef.c_str(), 0, 0, &x, &y, &w, &h);

  gfx->fillRect( x, y, w, h, wfMain_BackgroundBlue); // Clear to background

  gfx->setCursor( (gfx->width() - w) / 2, 135 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( mef );
}

// Draws current time for wfMain Timer

void TextMessageService::updateTimer( int minutesleft )
{
  gfx->setFont( &Minya_Nouvelle_Rg30pt7b );
  gfx->setTextSize( 0 );
  y = 135;
  String mef = (String) minutesleft;
  gfx->getTextBounds( mef.c_str(), 0, 0, &x, &y, &w, &h);

  gfx->fillRect( x, y, w, h, wfMain_BackgroundBlue); // Clear to background

  gfx->setCursor( (gfx->width() - w) / 2, 135 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( minutesleft );
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

boolean TextMessageService::fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font )
{
  if ( fadeset )
  {
    fadeset = 0;
    gfx->setFont( font );
    gfx->setTextSize(1);
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

    uint16_t r = map( fadestep, 0, steps, (backcolor >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (backcolor >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, backcolor & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    gfx->setFont( font );
    gfx->setTextSize(1);
    gfx->setCursor( (gfx->width() - w) / 2, y );
    gfx->setTextColor( textColor );
    gfx->println( text );
  }

  return false;
}

boolean TextMessageService::fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font )
{
  if ( fadeset )
  {
    fadeset = 0;

    gfx->setFont( font );
    gfx->setTextSize(1);
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

    uint16_t r = map( fadestep, 0, steps, (backcolor >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (backcolor >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, backcolor & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    gfx->setFont( font );
    gfx->setTextSize(1);
    gfx->setCursor( (gfx->width() - w) / 2, y );
    gfx->setTextColor( textColor );
    gfx->println( text );
  }

  return false;
}

// TextMessageService loop

void TextMessageService::loop()
{
  // Run the Show Time text animation

  if ( activated )
  {    
    if ( ( millis() - waitTime ) > stepDelay )
    {
      waitTime = millis();

      switch ( showNum ) 
      {
        case ShowDigitalTimeFunMessages:  // Fun messages fade around digital time
          runShowDigitalTimeFunMessages();
          break;
        case MysticalAnswer:  // Answers questions
          runMysticShow();
          break;
        case DigitalTime:     // Watch face main digital time
          runDigitalTime();
          break;
      }
    }
  }    

}
