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
extern GPS gps;
extern Accelerometer accel;
extern Video video;

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

// Maker Faire Mystic Answers

const char* mysticanswers[28][3] = {
    { "Oh, indeed",  "yes", "silly you!" },
    { "Absolutely",  "yes", "obviously." },
    { "A time for",  "change.", "Yes dear yes" },
    { "Yes,",        "without", "question." },
    { "Imagine",     "reality", "yes most yes" },
    { "My reality",  "not the same", "still yes" },
    { "It's certain,", "yes", "what else?" },
    { "Indubitably,", "yes", "my dear!" },
    { "Affirmative,", "yes", "surprisingly." },
    { "Figure it",   "out, or not!", "Do it today" },
    { "Not crazy,",  "just don't", "see it yet" },
    { "Adventures",  "require", "a first step" },
    { "Haste makes", "waste", "Try again" },
    { "Not all who", "wander", "are lost" },
    { "I prefer",    "the short-cut", "now" },
    { "If you don't", "know, neither", "do I" },
    { "Proper order?", "It's a mystery", "to me" },
    { "I can't",     "know", "everything" },
    { "Clear as",    "a clouded", "night" },
    { "Yes...",      "or no.", "Maybe both!" },
    { "Where's fun", "in knowing?", "huh!" },
    { "Think you're", "getting away", "with it? Nope." },
    { "Play fair?", "Not today", "Nope" },
    { "I am not all", "there myself,", "so no, no no" },
    { "No time for", "joy, nor", "play, no" },
    { "Right on", "time, just", "not today" },
    { "Hidden in", "plain sight.", "Right there" },
    { "We're all", "mad here,", "dear." },
};

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

void TimeService::begin()
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
  mystictype = 1;
}

bool TimeService::test()
{
  return true;
}

boolean TimeService::fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font)
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

    uint16_t r = map( fadestep, 0, steps, (backcolor >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (backcolor >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, backcolor & 0x1F, color & 0x1F);

    uint16_t textColor = (r << 11) | (g << 5) | b;

    gfx->setCursor( (gfx->width() - w) / 2, y );
    gfx->setTextColor( textColor );
    gfx->println( text );
  }

  return false;
}

boolean TimeService::fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font)
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

    uint16_t r = map( fadestep, 0, steps, (backcolor >> 11) & 0x1F, (color >> 11) & 0x1F);
    uint16_t g = map( fadestep, 0, steps, (backcolor >> 5) & 0x3F, (color >> 5) & 0x3F);
    uint16_t b = map( fadestep, 0, steps, backcolor & 0x1F, color & 0x1F);

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

  ShowTimeWaitTime = millis();
  stepDelay = 100;
}

bool TimeService::isTimeSet()
{
  if ( theTime == "0 o'clock" ) return false;
  return true;
}

void TimeService::runShowTellTime()
{
  if ( showStep == 0 )
  {    
    gfx->invertDisplay( true );
    //gfx->fillScreen( COLOR_BACKGROUND );
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

int TimeService::getMysticType()
{
  return mystictype;
}

// For Mystic Message from Maker Faire 2024 demo

void TimeService::runMysteryMessage()
{
  if ( showStep == 0 )
  {    
    showStep = 1;
    fadeset = 1;

    int index = random(0, 28);

    if ( index < 10 ) 
    {
      mystictype = 1;
    } else if ( index < 19) {
      mystictype = 2;
    } else {
      mystictype = 3;
    }

    theMsg1 = mysticanswers[ index ][ 0 ];
    theMsg2 = mysticanswers[ index ][ 1 ];
    theMsg3 = mysticanswers[ index ][ 2 ];

    Serial.print( "runMysteryMessage: " );
    Serial.print( theMsg1 );
    Serial.print( ", " );
    Serial.print( theMsg2 );
    Serial.print( ", " );
    Serial.println( theMsg3 );

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
    if ( fadeInCenteredText( theMsg2, 130, 50, COLOR_STRIPE_MEDIUM_GRAY, COLOR_BLACK, &Minya16pt7b ) )
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
   if ( fadeInCenteredText( theMsg3, 170, 125, COLOR_STRIPE_PINK, COLOR_BLACK, &ScienceFair14pt7b ) )
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
   if ( fadeOutCenteredText( theMsg3, 170, 75, COLOR_STRIPE_PINK, COLOR_BLACK, &ScienceFair14pt7b ) )
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
    if ( fadeOutCenteredText( theMsg2, 130, 20, COLOR_STRIPE_MEDIUM_GRAY, COLOR_BLACK, &Minya16pt7b ) )
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
    if ( fadeOutCenteredText( theMsg1, 90, 20, COLOR_TEXT_YELLOW, COLOR_BLACK, &Some_Time_Later20pt7b ) )
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

// For WatchFace shows digital time

void TimeService::runDigitalTime()
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

bool TimeService::getTimeAnimationActivated()
{
  return activated;
}

void TimeService::setTimeAnimationActivated( bool act )
{
  activated = act;
}

bool TimeService::getDialActivated()
{
  return dialActivated;
}

void TimeService::setDialActivated( bool set )
{
  dialActivated = set;

  if ( set )
  {
    drawClockFace();
    setCarrotAngleFromRTC();
  } 
}

String TimeService::getTimeStringFromAngle( float angle )
{
  angle -= 90;
  if (angle < 0) angle += 360;

  int hour, minute;
  getTimeFromAngle(angle, hour, minute);
  
  if ( hour == 0 ) hour = 12;
  if ( minute == 44 ) minute = 45;
  if ( minute == 14 ) minute = 15;

  // Format the time as a string
  char timeString[6]; // Buffer to hold the formatted time string (HH:MM\0)
  sprintf( timeString, "%2d:%02d", hour, minute );
  return String( timeString );
}

void TimeService::setCarrotAngleFromRTC() 
{
  // Assuming RTC is already initialized and set
  struct tm timeinfo;

  if ( !getLocalTime( &timeinfo ) ) 
  {
    // Serial.println( "TimeService RTC not set" );
    carrotAngle = 0;
    drawCarrot( carrotAngle );
    return;
  }
  
  int hour = timeinfo.tm_hour % 12; // Convert to 12-hour format
  int minute = timeinfo.tm_min;
  
  // Calculate angle: Each hour is 30 degrees and each minute is 0.5 degrees
  carrotAngle = (hour * 30) + (minute * 0.5);
  
  // Adjust for the display rotation
  carrotAngle -= 90;
  if (carrotAngle < 0) carrotAngle += 360;

  // Draw the carrot at the calculated angle
  drawCarrot(carrotAngle);
  
  // Debug output
  Serial.print("TimeService RTC reports: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  Serial.print("Carrot angle set to: ");
  Serial.println(carrotAngle);
}

void TimeService::showDigitalTime()
{ 

  if ( ! ( pastTimeStr == getTimeStringFromAngle( carrotAngle ) ) )
  {
    gfx->setFont( &Some_Time_Later20pt7b );
    gfx->getTextBounds( pastTimeStr.c_str(), 0, 0, &x, &y, &w, &h);    
    gfx->setCursor( (gfx->width() - w) / 2, 120 );
    gfx->setTextColor( COLOR_BLACK );
    gfx->println( pastTimeStr );
  }

  pastTimeStr = getTimeStringFromAngle( carrotAngle );

  gfx->setFont( &Some_Time_Later20pt7b );
  gfx->getTextBounds( pastTimeStr.c_str(), 0, 0, &x, &y, &w, &h);    
  gfx->setCursor( (gfx->width() - w) / 2, 120 );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( pastTimeStr );

  String phrase = "Play time?";
  gfx->setFont( &ScienceFair14pt7b );
  gfx->getTextBounds( phrase.c_str(), 0, 0, &x, &y, &w, &h);    
  gfx->setCursor( (gfx->width() - w) / 2, 160 );
  gfx->setTextColor( COLOR_STRIPE_MEDIUM_GRAY );
  gfx->println( phrase );
}

void TimeService::drawClockFace() 
{
  // Draw hour tick marks

  for (int i = 0; i < 12; i++) 
  {
    float angle2 = i * 30.0;  // 360 degrees / 12 hours = 30 degrees per hour mark
    for (int j = 0; j <= 2; j++) 
    { // 4 pixels wide line
      float rad_angle = (angle2 + j) * PI / 180;
      int x1 = CENTER_X + RADIUS * cos(rad_angle);
      int y1 = CENTER_Y + RADIUS * sin(rad_angle);
      int x2 = CENTER_X + (RADIUS - 20) * cos(rad_angle);  // Shorter tick marks for hours
      int y2 = CENTER_Y + (RADIUS - 20) * sin(rad_angle);
      gfx->drawLine(x1, y1, x2, y2, COLOR_STRIPE_MEDIUM_GRAY);
    }
  }
  
  // Draw quarter hour tick marks
  for (int i = 1; i < ( 12 * 4 ); i++)
  {
    float rad_angle = ( i * 7.5 ) * PI / 180;
    int x1 = CENTER_X + RADIUS * cos(rad_angle);
    int y1 = CENTER_Y + RADIUS * sin(rad_angle);
    int x2 = CENTER_X + (RADIUS - 10) * cos(rad_angle);  // Shorter tick marks for hours
    int y2 = CENTER_Y + (RADIUS - 10) * sin(rad_angle);
    gfx->drawLine(x1, y1, x2, y2, COLOR_STRIPE_PINK);
  }
}

void TimeService::drawCarrot( float angle ) 
{
  static float lastAngle = 0;
  
  // Adjust for the display rotation
  angle -= 90;
  if (angle < 0) angle += 360;

  if ( prior )
  {
    // Erase the previous carrot
    float lastX = SCREEN_WIDTH / 2 + cos(lastAngle * DEG_TO_RAD) * CARROT_MOVEMENT_RADIUS;
    float lastY = SCREEN_HEIGHT / 2 + sin(lastAngle * DEG_TO_RAD) * CARROT_MOVEMENT_RADIUS;
    gfx->fillCircle(lastX, lastY, CARROT_RADIUS, COLOR_BLACK);
  }

  float x = SCREEN_WIDTH / 2 + cos(angle * DEG_TO_RAD) * CARROT_MOVEMENT_RADIUS;
  float y = SCREEN_HEIGHT / 2 + sin(angle * DEG_TO_RAD) * CARROT_MOVEMENT_RADIUS;
  gfx->fillCircle(x, y, CARROT_RADIUS, COLOR_STRIPE_PINK);

  prior = true;  
  lastAngle = angle;  
}

void TimeService::getTimeFromAngle(float angle, int &hour, int &minute) 
{
  // Adjust for the display rotation
  angle += 90;
  if (angle >= 360) angle -= 360;
  
  hour = ((int)angle / 30) % 12;
  if ( hour == 0 ) hour = 12;

  minute = ((int)angle % 30) * 2;
}

void TimeService::updateSetTime()
{
  if ( ! dialActivated ) return;

  float x = accel.getXaccelReading();

  // Calculate carrot speed and direction
  if ( abs(x) > 0.01 ) 
  {
    carrotAngle += ( x * 500 ); // Adjust the multiplier to change sensitivity
    lastMoveTime = millis();
  }
  else
  {
    // Check for timeout
    if ( millis() - lastMoveTime > SETTIME_TIMEOUT )
    {
      int hour, minute;

      float finangle = carrotAngle - 90;
      if ( finangle < 0 ) finangle += 360;

      getTimeFromAngle( finangle, hour, minute );

      Serial.print("SetTime finished with ");
      Serial.print( hour );
      Serial.print( ":" );
      Serial.println( minute );

      setDialActivated( false );
      return;
    }
  }
  
  // Keep carrotAngle within 0-360 degrees
  if (carrotAngle < 0) carrotAngle += 360;
  if (carrotAngle >= 360) carrotAngle -= 360;
  
  // Snap to 0, 15, 30, 45 minute positions
  int markerAngle = round( carrotAngle / 7.5) * 7.5;
  carrotAngle = markerAngle;

  drawCarrot( carrotAngle );
  showDigitalTime();
  //video.drawIcons();
}

// TimeService loop

void TimeService::loop()
{
  // Run the Show Time text animation

  if ( activated )
  {    
    if ( ( millis() - ShowTimeWaitTime ) > stepDelay )
    {
      ShowTimeWaitTime = millis();

      // Show the time

      if ( showNum == 0 ) runShowTellTime();

      if ( showNum == 1 ) runDigitalTime();

      if ( showNum == 2 ) 
      {
        runMysteryMessage();
      }
      
    }
  }

  // Run the Set Time animation

  if ( dialActivated )
  {
    if ( ( millis() - stTime ) > 350 )
    {
      stTime = millis();

      updateSetTime();
    }
  }
}
