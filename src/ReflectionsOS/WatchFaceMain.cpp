/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Requires PNGdec library https://github.com/bitbank2/PNGdec

*/

#include "WatchFaceMain.h"

// -----------------------------------------------------------------------------
// Shared state for Set Time + Confirm Set Time behavior
// -----------------------------------------------------------------------------
static int g_confirmHour = 0;
static int g_confirmMin  = 0;

static int g_lastSigHour = -1;
static int g_lastSigMin  = -1;

// Digital Time tap gate: clear taps for 2 seconds on entry
static uint32_t g_displayTimeTapGateStart = 0;
static const uint32_t DISPLAYTIME_TAP_GATE_MS = 2000;

// Wrap-safe minute diff, e.g. 59 -> 0 is diff 1
static int minuteDiffWrap(int a, int b)
{
  int d = abs(a - b);
  if (d > 30) d = 60 - d;
  return d;
}

// -----------------------------------------------------------------------------
// Twist helper (resettable)
// -----------------------------------------------------------------------------
static uint32_t g_twistLastPoll = 0;
static uint32_t g_twistLastFire = 0;

static void resetTwistDetector()
{
  // Prevent any immediate fire right after a panel change
  g_twistLastPoll = millis();
  g_twistLastFire = millis();
}

static void flushTwistDetector()
{
  // Drain any internal "2-hit" state in AccelSensor by reading it
  (void)accel.getWristTwistDir();
}

static bool twistTriggered()
{
  // Poll at a calm rate
  if (millis() - g_twistLastPoll < 300) return false;
  g_twistLastPoll = millis();

  // Extra cooldown
  if (millis() - g_twistLastFire < 900) return false;

  int dir = accel.getWristTwistDir();
  if (dir != 0)
  {
    g_twistLastFire = millis();
    return true;
  }
  return false;
}

WatchFaceMain::WatchFaceMain()
  : WatchFaceBase() // Call to the base class constructor
{}

void WatchFaceMain::begin()
{
  WatchFaceBase::begin();  // This ensures the base method is executed

  maintimer = millis();

  drawitall = true;
  drawFace = false;

  catFaceIndex = 1;
  catFaceTimer = millis();
  catFaceDirection = true;
  catFaceWait = rand() % 30000;

  tilttimer = millis();

  // Baseline ranges (used elsewhere, and as a reference)
  smallX = -6;
  largeX = 6;
  smallY = 7;
  largeY = -7;

  battimer = millis();
  batcount = battery.getBatteryLevel();
  batlev = 0;

  oldHour = 0;
  oldMinute = 0;
  oldBattery = 5;
  oldBlink = 1;

  gpsflag = false;
  gpsx = 0;
  gpsy = 0;
  gpsdirection = 0;

  notificationflag = false;

  slowman = millis();

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  panel = STARTUP;

  textmessageservice.deactivate();

  temptimer = millis();

  minuteRedrawtimer = millis();
  sleepyTimer = millis();
  sleepyTimer2 = millis();

  movesOld = 0;

  mainwaiter = millis();

  _runmode = true;

  signaledSleepy = false;
}

void WatchFaceMain::clearSleepy()
{
  sleepyTimer = millis();
  sleepyTimer2 = millis();
  signaledSleepy = false;
}

bool WatchFaceMain::isSleepy()
{
  if ( millis() - sleepyTimer < sleepyTime )
  {
    return false;
  }
  else
  {
    if ( ! signaledSleepy )
    {
      signaledSleepy = true;
      return true;
    }
    else
    {
      return false;
    }
  }  
}

bool WatchFaceMain::goToSleep()
{
  if ( millis() - sleepyTimer2 < sleepyTime2 ) return false;
  return true;
}

void WatchFaceMain::setDrawItAll()
{
  drawitall = true;
}

// helper: map float
float WatchFaceMain::mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// GPS marker
void WatchFaceMain::updateGPSmarker()
{
  if ( gps.isActive() )
  {
    float gpsCourse = gps.getCourse(); // Course angle in degrees

    if ( gpsdirection > 360 ) gpsdirection = 0;

    int imageSize = 24; // Image size (24x24)
    int centerX = 120; // Center of the 240x240 screen
    int centerY = 120;
    int radius = 100;

    float angleRad = radians( gpsCourse );

    // Calculate the position to place the top-left corner of the image
    gpsx = centerX + radius * cos(angleRad) - (imageSize / 2);
    gpsy = centerY + radius * sin(angleRad) - (imageSize / 2);
  }
}

// Timer notice indicator
void WatchFaceMain::updateTimerNotice()
{
  notificationflag = timerservice.status();
}

// Battery indicator grows/loses leaves
void WatchFaceMain::updateBattery()
{
  if ( ( millis() - battimer ) > ( 2 * 60000 ) )
  {
    battimer = millis();

    batlev = battery.getBatteryLevel();

    if ( ( batlev != batcount ) && ( batcount < 5 ) )
    {
      batcount = batlev;
      displayUpdateable = true;
    }
  }
}

// Hour and minutes hands to current time
void WatchFaceMain::updateHoursAndMinutes()
{
  int hour2 = realtimeclock.getHour();
  int minute2 = realtimeclock.getMinute();

  if ( hour2 != hour )
  {
    hour = hour2;
    displayUpdateable = true;
    drawFace = true;
  }

  if ( minute2 != minute )
  {
    minute = minute2;
    displayUpdateable = true;
    drawFace = true;
  }
}

// Cat blinks eyes
void WatchFaceMain::updateBlink()
{
  if ( ! blinking )
  {
    if ( ( millis() - catFaceTimer ) > catFaceWait )
    {
      catFaceTimer = millis();
      blinking = true;
      catFaceDirection = true;
    }
  }
  else
  {
    if ( catFaceDirection )
    {
      catFaceIndex++;
      displayUpdateable = true;

      if ( catFaceIndex > 5 )
      {
        catFaceDirection = false;
        catFaceIndex = 5;
      }
    }
    else
    {
      catFaceIndex--;
      displayUpdateable = true;

      if ( catFaceIndex < 1 )
      {
        catFaceDirection = true;
        catFaceIndex = 1;
        blinking = false;
        catFaceTimer = millis();
        catFaceWait = rand() % 30000;
      }
    }
  }
}

// Draw the elements to the display
void WatchFaceMain::showDisplayMain()
{
  if ( ! displayUpdateable ) return;
  displayUpdateable = false;

  if ( drawitall ) drawImageFromFile( wfMainBackground, true, 0, 0 );

  String mef;

  // Draw hour image
  if ( ( hour != oldHour ) || drawitall )
  {
    oldHour = hour;

    if ( hour > 12 ) hour = 12;   // Just to be safe

    mef = wfMainHours;
    mef += hour;
    mef += wfMainHours2;
    drawImageFromFile( mef, true, 0, 0 );
  }

  if ( ( minute != oldMinute ) || drawitall )
  {
    oldMinute = minute;
    int mymin = map( minute, 1, 60, 0, 23);
    if ( mymin > 23 ) mymin = 23;   // Just to be safe

    mef = wfMainMinutes;
    mef += mymin ;
    mef += wfMainMinutes2;
    drawImageFromFile( mef, true, 0, 0 );
  }

  // Battery indicator
  if ( ( batcount != oldBattery ) || drawitall )
  {
    oldBattery = batcount;

    mef = wfMainBattery;
    mef += batcount;
    mef += wfMainBattery2;
    drawImageFromFile( mef, true, 0, 0 );
  }

  // Timer indicator
  if ( notificationflag )
  {
    mef = wfMain_Face_Main_TimerNotice;
    drawImageFromFile( mef, true, 0, 0 );
  }

  // Cat face in the middle, and he blinks randomly
  if ( ( catFaceIndex != oldBlink ) || drawitall || drawFace )
  {
    oldBlink = catFaceIndex;
    drawFace = false;

    mef = wfMainFace;
    mef += catFaceIndex;
    mef += wfMainFace2;
    drawImageFromFile( mef, true, 0, 0 );
  }

  if ( gpsflag )
  {
    mef = wfMain_GPSmarker;
    drawImageFromFile( mef, true, gpsx, gpsy );
  }

  show();

  drawitall = false;
}

// Draws hour glass time left display, 3, 2, 1
// Returns true when time is up
bool WatchFaceMain::updateTimeLeft()
{
  if ( panel == MAIN ) return false;

  int index = 1;

  if ( ( millis() - noMovementTime ) > ( nomov * 1 ) ) index = 2;
  if ( ( millis() - noMovementTime ) > ( nomov * 2 ) ) index = 3;
  if ( ( millis() - noMovementTime ) > ( nomov * 3 ) ) index = 4;

  if ( ( millis() - noMovementTime ) > ( nomov * 4 ) )
  {
    return true;
  }

  mef = wfMainHourglass;
  mef += index;
  mef += wfMainHourglass2;

  drawImageFromFile( mef, true, 0, 0 );

  show();

  return false;
}

void WatchFaceMain::printStatus()
{
  if ( millis() - temptimer > 2000 )
  {
    temptimer = millis();

    String mef = "WatchFaceMain ";

    if ( video.getStatus() ) mef += "1 ";
    else mef += "0 ";

    if ( textmessageservice.active() ) mef += "1 ";
    else mef += "0 ";

    Serial.println( mef );
  }

  return;
}

bool WatchFaceMain::isMain()
{
  if ( panel == MAIN ) return true;
  return false;
}

bool WatchFaceMain::isMainOrTime()
{
  if ( panel == MAIN ) return true;
  if ( panel == DISPLAYING_TIME ) return true;
  return false;
}


void WatchFaceMain::loop()
{
  if ( video.getStatus() ) return;

  switch ( panel )
  {
    case STARTUP:
      startup();
      break;

    case MAIN:
      main();
      break;

    case DISPLAYING_TIME:
      displaytime();
      break;

    case SETTING_TIME:
      settingtime();
      break;

    case CONFIRM_SETTING_TIME:
      confirmsettingtime();
      break;

    case DISPLAYING_MOVES:
      displayingmoves();
      break;

    case CONFIRM_CLEAR_MOVES:
      confirmclearmoves();
      break;

    case DISPLAYING_TIMER:
      displayingtimer();
      break;

    case SETTING_TIMER:
      settingtimer();
      break;

    case CONFIRM_START_TIMER:
      confirmstarttimer();
      break;
  }

  // Timeout any panel - except for the main - with no change in 20 seconds
  if ( ( ( millis() - noMovementTime ) > 20000 ) && ( panel != MAIN ) )
  {
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  // Force a redraw periodically, to move the minutes hand
  if ( millis() - minuteRedrawtimer > 100000 )
  {
    setDrawItAll();
    showDisplayMain();
    minuteRedrawtimer = millis();
  }
}

bool WatchFaceMain::isSettingTime()
{
  if ( ( panel == SETTING_TIME ) || ( panel == CONFIRM_SETTING_TIME ) ||
       ( panel == SETTING_TIMER ) || ( panel == CONFIRM_START_TIMER ) ) return true;

  return false;
}

void WatchFaceMain::changeTo( int panelnum, bool setup, String videoname )
{
  panel = panelnum;
  needssetup = setup;
  textmessageservice.stop();
  if ( videoname != "none" ) video.startVideo( videoname );
  noMovementTime = millis();
  clearSleepy();
}

void WatchFaceMain::startup()
{
  panel = MAIN;
  needssetup = true;
  blinking = false;
  textmessageservice.deactivate();
  clearSleepy();
}

void WatchFaceMain::main()
{
  if ( needssetup )
  {
    Serial.println( F("MAIN") );
    needssetup = false;
    drawitall = true;
    blinking = false;
    mainwaiter = millis();
    return;
  }

  // MAIN: Twist => Digital Time panel
  if ( twistTriggered() && ( millis() - mainwaiter > 2000 ) )
  {
    changeTo( DISPLAYING_TIME, true, WatchFaceFlip1_video );
    return;
  }

  // Single tap does nothing on MAIN (per your flow)

  if ( ( millis() - maintimer > 50 ) && ( ! video.getStatus() ) )
  {
    maintimer = millis();

    updateBlink();            // Blink eyes
    updateBattery();          // Battery level indicator
    updateHoursAndMinutes();  // Hour and minute hands
    // updateTimerNotice();   // Timer notice (disabled for shipment)
    // updateGPSmarker();     // GPS marker (disabled for shipment)

    showDisplayMain();
  }
}

// -----------------------------------------------------------------------------
// Digital Time: tap -> suppress twist window
// -----------------------------------------------------------------------------

static uint32_t g_lastTapMs = 0;
static const uint32_t TAP_SUPPRESS_TWIST_MS = 500;

// at top of file (near your other statics)
static bool g_displayTimeDrained = false;

void WatchFaceMain::displaytime()
{
  if ( needssetup )
  {
    needssetup = false;

    Serial.println( F("DISPLAYING_TIME") );
    noMovementTime = millis();

    g_displayTimeTapGateStart = millis();
    g_displayTimeDrained = false;

    // Drain stale events ONCE on entry (from MAIN / transition)
    (void)accel.getSingleTap();
    (void)accel.getDoubleTap();
    g_displayTimeDrained = true;

    drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
    textmessageservice.startShow( TextMessageExperiences::DigitalTime, "", "" );
    return;
  }

  // Keep the 2 second settle, but DON'T drain during it.
  if ( millis() - g_displayTimeTapGateStart < DISPLAYTIME_TAP_GATE_MS )
  {
    return;
  }

  // After settle: Double tap => enter SETTING_TIME
  if ( accel.getDoubleTap() )
  {
    Serial.println("Displaying_Time got a double tap" );
    changeTo( SETTING_TIME, true, "none" );
    return;
  }

  // After settle: Twist => back to MAIN
  if ( twistTriggered() )
  {
    Serial.println("Displaying_Time got a Twist" );
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  // Single tap does nothing here
}

void WatchFaceMain::settingtime()
{
  // More sensitive tilt ranges for setting time (shorter wrist angles have more effect)
  // Smaller absolute values => more sensitive.
  const float setSmallX = -3.0f;
  const float setLargeX =  3.0f;
  const float setSmallY =  3.5f;
  const float setLargeY = -3.5f;

  if ( needssetup )
  {
    Serial.println( F("SETTING_TIME") );
    needssetup = false;
    noMovementTime = millis();
    tilttimer = millis();

    hour = realtimeclock.getHour();
    minute = realtimeclock.getMinute();

    // Initialize “significant change” tracking
    g_lastSigHour = hour;
    g_lastSigMin  = minute;

    // Initialize confirm (frozen) values
    g_confirmHour = hour;
    g_confirmMin  = minute;

    drawHourMinute( hour, minute );
    return;
  }

  clearSleepy();

  // SETTING_TIME: Twist => back to MAIN
  /* Disabled so a Twist or accidental click won't interrupt setting time
  if ( twistTriggered() )
  {
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }
  */

  // Hourglass timeout drives confirm panel
  if ( updateTimeLeft() )
  {
    // Freeze chosen time at the moment we leave SETTING_TIME
    g_confirmHour = hour;
    g_confirmMin  = minute;

    changeTo( CONFIRM_SETTING_TIME, true, "none" );
    return;
  }

  // Single tap does nothing on SETTING_TIME (per your flow)
  // Double tap does nothing here too.

  // Set time:
  // horizontal (X) adjusts minutes, vertical (Y) adjusts hours
  if ( millis() - tilttimer < tiltspeed ) return;
  tilttimer = millis();

  float xraw = accel.getXreading();
  float yraw = accel.getYreading();

  int newMinute = constrain( round( mapFloat( xraw, setSmallX, setLargeX, 0.0f, 59.0f ) ), 0, 59 );
  int newHour   = constrain( round( mapFloat( yraw, setLargeY, setSmallY, 1.0f, 12.0f ) ), 1, 12 );

  bool changed = false;

  if ( minute != newMinute )
  {
    minute = newMinute;
    changed = true;
  }

  if ( hour != newHour )
  {
    hour = newHour;
    changed = true;
  }

  if ( changed )
  {
    // Only reset hourglass if change is "significant":
    // - hour changed, OR minutes moved by >= 5 (wrap-safe)
    bool significant = false;

    if (hour != g_lastSigHour) significant = true;
    if (minuteDiffWrap(minute, g_lastSigMin) >= 5) significant = true;

    if (significant)
    {
      noMovementTime = millis();
      g_lastSigHour = hour;
      g_lastSigMin  = minute;
    }

    // Always redraw on any change
    drawHourMinute( hour, minute );

    // Track the most recent chosen values for confirmation
    g_confirmHour = hour;
    g_confirmMin  = minute;
  }
}

void WatchFaceMain::drawHourMinute( int hourc, int minutec )
{
  String mef = String( hourc );
  mef += F(":");
  if ( minutec < 10 ) mef += F("0");
  mef += String( minutec );

  drawImageFromFile( wfMain_SetTime_Background_Shortie, true, 0, 0 );
  textmessageservice.updateTempTime( mef );
}

void WatchFaceMain::confirmsettingtime()
{
  if ( needssetup )
  {
    Serial.println( F("CONFIRM_SETTING_TIME") );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  // If user does nothing, return to MAIN
  if ( updateTimeLeft() )
  {
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  // CONFIRM_SETTING_TIME: Single tap accepts and sets RTC
  if ( accel.getSingleTap() || accel.getDoubleTap() )
  {
    Serial.printf("Accepted new time setting: %d:%02d\n", g_confirmHour, g_confirmMin);
    realtimeclock.setHourMinute(g_confirmHour, g_confirmMin);
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  textmessageservice.drawCenteredMesssage( F("Tap To"), F("Set Time") );
}

void WatchFaceMain::displayingmoves()
{
  if ( needssetup )
  {
    Serial.println( F("DISPLAYING_MOVES") );
    needssetup = false;
    drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
    noMovementTime = millis();
    movesflag = true;
    return;
  }

  if ( accel.getDoubleTap() )
  {
    changeTo( CONFIRM_CLEAR_MOVES, true, "none" );
    return;
  }

  if ( accel.getSingleTap() )
  {
    changeTo( DISPLAYING_TIMER, true, WatchFaceFlip2_video );
    return;
  }

  int newmoves = steps.howManySteps();
  if ( newmoves != movesOld )
  {
    movesOld = newmoves;
    drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
    textmessageservice.updateHealth( newmoves );
  }
}

void WatchFaceMain::confirmclearmoves()
{
  if ( needssetup )
  {
    Serial.println( F("CONFIRM_CLEAR_MOVES") );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( updateTimeLeft() )
  {
    changeTo( DISPLAYING_TIMER, true, WatchFaceFlip2_video );
    return;
  }

  if ( accel.getSingleTap() || accel.getDoubleTap() )
  {
    steps.resetStepCount();
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  textmessageservice.drawCenteredMesssage( F("Tap To"), F("Clear Moves") );
}

void WatchFaceMain::displayingtimer()
{
  if ( needssetup )
  {
    Serial.println( F("DISPLAYING_TIMER") );
    needssetup = false;
    drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.getSingleTap() )
  {
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  if ( accel.getDoubleTap() )
  {
    changeTo( SETTING_TIMER, true, "none" );
    return;
  }

  textmessageservice.updateTimer( timerservice.getTime() );
}

void WatchFaceMain::settingtimer()
{
  if ( needssetup )
  {
    Serial.println( F("SETTING_TIMER") );
    needssetup = false;
    drawImageFromFile( wfMain_SetTimer_Background, true, 0, 0 );
    noMovementTime = millis();
    hour = timerservice.getTime();
    tilttimer = millis();
    return;
  }

  if ( updateTimeLeft() )
  {
    changeTo( CONFIRM_START_TIMER, true, "none" );
    return;
  }

  if ( accel.getSingleTap() )
  {
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  // Change timer time by tilting up-down
  if ( millis() - tilttimer < tiltspeed ) return;
  tilttimer = millis();

  float yraw = accel.getYreading();

  int newHours = constrain( round( mapFloat(yraw, largeY, smallY, 1.0f, 12.0f) ), 1, 12 );

  if ( hour != newHours )
  {
    hour = newHours;
    noMovementTime = millis();  // Reset inactivity timer
    hourschanging = false;
    timerservice.setTime( hour );
    String met = String( hour );
    drawImageFromFile( wfMain_SetTimer_Background_Shortie, true, 0, 0 );
    textmessageservice.updateTempTime( met );
    return;
  }
}

void WatchFaceMain::confirmstarttimer()
{
  if ( needssetup )
  {
    Serial.println( F("CONFIRM_START_TIMER") );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( updateTimeLeft() )
  {
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  if ( ( accel.getSingleTap() || accel.getDoubleTap() ) && ( noMovementTime > 5000 ) )
  {
    timerservice.start();
    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  textmessageservice.drawCenteredMesssage( F("Tap To"), F("Start Timer") );
}