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
static void clearSetTimeExitNoise();

static int g_confirmHour = 0;
static int g_confirmMin  = 0;

static int g_lastSigHour = -1;
static int g_lastSigMin  = -1;

// Set Time protected mode blocks all experiences/BLE/sleep interruptions until
// the return-to-main transition video has finished.
static bool g_setTimeProtected = false;

// Twist-and-hold entry: first twist arms a 3s window.
// Reverse/second twist before 3s means normal Digital Time.
// Holding the twist for 3s means Set Time.
static bool g_twistHoldArmed = false;
static uint32_t g_twistHoldStartedAt = 0;
static const uint32_t TWIST_HOLD_SET_TIME_MS = 1800;

// Digital Time tap gate: clear taps for 2 seconds on entry
static uint32_t g_displayTimeTapGateStart = 0;
static const uint32_t DISPLAYTIME_TAP_GATE_MS = 2000;

// Set Time tilt gate: one accepted change requires tilt -> stable hold -> neutral.
// Set Time tilt auto-repeat.
// First accepted change requires stable tilt.
// Holding the tilt repeats changes at SETTIME_REPEAT_MS.
// Returning to center resets the repeat.
static int g_setTimeTiltDirection = 0;
static uint32_t g_setTimeTiltStartedAt = 0;
static uint32_t g_setTimeTiltLastStepAt = 0;

static const float SETTIME_TILT_THRESHOLD = 3.0f;
static const float SETTIME_NEUTRAL_THRESHOLD = 1.4f;
static const uint32_t SETTIME_INITIAL_STABLE_MS = 300;
static const uint32_t SETTIME_REPEAT_MS = 300;
static const uint32_t SETTIME_HOUR_REPEAT_MS = 375;  // Hours change 25% slower than minutes

static uint32_t g_ignoreTofUntil = 0;

// -----------------------------------------------------------------------------
// Orientation-driven twist / TOF gating
// -----------------------------------------------------------------------------
// The normal/upright orientation is captured when the MAIN watch face starts.
// TOF gestures are only allowed when the watch is within about +/- 10 degrees
// of that upright orientation.
//
// Twist behavior:
//   - Leave upright: arm the gesture.
//   - Return to upright before Set Time hold completes: show Digital Time.
//   - Hold more than 90 degrees away from upright for 2 seconds: enter Set Time.

static float g_uprightX = 0;
static float g_uprightY = 0;
static float g_uprightZ = 0;
static bool g_uprightCaptured = false;

// TOF has its own captured upright orientation. This keeps the TOF gate
// independent from the orientation gesture state machine.
static float g_tofUprightX = 0;
static float g_tofUprightY = 0;
static float g_tofUprightZ = 0;
static bool g_tofUprightCaptured = false;

static uint32_t g_setTimePast90StartedAt = 0;

static const float ORIENTATION_UPRIGHT_DOT_THRESHOLD = 0.985f;  // cos(10 degrees)
static const float ORIENTATION_ARM_DOT_THRESHOLD     = 0.50f;   // about 60 degrees away
static const float ORIENTATION_SETTIME_DOT_THRESHOLD = 0.00f;   // more than 90 degrees away
static const uint32_t ORIENTATION_SETTIME_HOLD_MS = 2000;

static bool readAccelUnitVector(float &x, float &y, float &z)
{
  x = accel.getXreading();
  y = accel.getYreading();
  z = accel.getZreading();

  float mag = sqrt((x * x) + (y * y) + (z * z));
  if (mag < 0.01f) return false;

  x /= mag;
  y /= mag;
  z /= mag;

  return true;
}

static void captureUprightOrientation()
{
  float x, y, z;
  if (!readAccelUnitVector(x, y, z))
  {
    g_uprightCaptured = false;
    Serial.println(F("MAIN: unable to capture upright orientation"));
    return;
  }

  g_uprightX = x;
  g_uprightY = y;
  g_uprightZ = z;
  g_uprightCaptured = true;

  Serial.printf("MAIN: upright captured x=%0.2f y=%0.2f z=%0.2f\n", x, y, z);
}

static void captureTofUprightOrientation()
{
  float x, y, z;
  if (!readAccelUnitVector(x, y, z))
  {
    g_tofUprightCaptured = false;
    Serial.println(F("TOF: unable to capture upright orientation"));
    return;
  }

  g_tofUprightX = x;
  g_tofUprightY = y;
  g_tofUprightZ = z;
  g_tofUprightCaptured = true;

  Serial.printf("TOF: upright captured x=%0.2f y=%0.2f z=%0.2f\n", x, y, z);
}

static float currentDotFromUpright()
{
  if (!g_uprightCaptured) return 1.0f;

  float x, y, z;
  if (!readAccelUnitVector(x, y, z)) return 1.0f;

  return (x * g_uprightX) + (y * g_uprightY) + (z * g_uprightZ);
}

static bool isCurrentOrientationNearlyUprightForTof()
{
  // If TOF upright has not been captured yet, allow TOF rather than
  // accidentally disabling all TOF gestures at startup.
  if (!g_tofUprightCaptured) return true;

  float x, y, z;
  if (!readAccelUnitVector(x, y, z)) return false;

  float dot =
    (x * g_tofUprightX) +
    (y * g_tofUprightY) +
    (z * g_tofUprightZ);

  // Within roughly +/- 10 degrees of the captured MAIN/upright position.
  // cos(10 degrees) = 0.9848
  return dot >= ORIENTATION_UPRIGHT_DOT_THRESHOLD;
}

static void resetOrientationGestureState()
{
  g_twistHoldArmed = false;
  g_setTimePast90StartedAt = 0;
}

bool WatchFaceMain::isNearlyUprightForTof()
{
  return isCurrentOrientationNearlyUprightForTof();
}

bool WatchFaceMain::shouldIgnoreTofGestures()
{
  return
    g_twistHoldArmed ||
    isSettingTime() ||
    !isNearlyUprightForTof();
}

static void resetSetTimeTiltGate()
{
  g_setTimeTiltDirection = 0;
  g_setTimeTiltStartedAt = 0;
  g_setTimeTiltLastStepAt = 0;
}

// Returns -1 for left tilt, +1 for right tilt, 0 for no step.
static int getSetTimeTiltStep(uint32_t repeatMs)
{
  const uint32_t now = millis();
  const float xraw = accel.getXreading();

  if (fabs(xraw) <= SETTIME_NEUTRAL_THRESHOLD)
  {
    resetSetTimeTiltGate();
    return 0;
  }

  int dir = 0;

  if (xraw >= SETTIME_TILT_THRESHOLD) dir = 1;
  else if (xraw <= -SETTIME_TILT_THRESHOLD) dir = -1;
  else return 0;

  // New tilt direction starts timing.
  if (dir != g_setTimeTiltDirection)
  {
    g_setTimeTiltDirection = dir;
    g_setTimeTiltStartedAt = now;
    g_setTimeTiltLastStepAt = 0;
    return 0;
  }

  // First step after stable hold.
  if (g_setTimeTiltLastStepAt == 0)
  {
    if (now - g_setTimeTiltStartedAt >= SETTIME_INITIAL_STABLE_MS)
    {
      g_setTimeTiltLastStepAt = now;
      return dir;
    }

    return 0;
  }

  // Auto-repeat while still tilted.
  if (now - g_setTimeTiltLastStepAt >= repeatMs)
  {
    g_setTimeTiltLastStepAt = now;
    return dir;
  }

  return 0;
}

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
  if (millis() - g_twistLastPoll < 200) return false;
  g_twistLastPoll = millis();

  // Extra cooldown
  if (millis() - g_twistLastFire < 700) return false;

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

  movesOld = 0;

  mainwaiter = millis();

  _runmode = true;
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
  // If we are returning from Set Time, keep experiences blocked until the
  // transition-to-main video has completely finished.
  if ( g_setTimeProtected && ( panel == MAIN ) && ( ! video.getStatus() ) )
  {
    Serial.println( F("SET_TIME: experiences re-enabled") );
    g_setTimeProtected = false;
  }

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

    case SETTING_MINUTE:
      settingminute();
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

  // Do not use the generic panel timeout while Set Time is active. Set Time
  // advances itself using the hourglass/updateTimeLeft() flow.
  if ( ( ( millis() - noMovementTime ) > 20000 ) && ( panel != MAIN ) && !isSettingTime() )
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
  if ( g_setTimeProtected ) return true;

  if ( ( panel == SETTING_TIME ) || ( panel == SETTING_MINUTE ) ||
       ( panel == CONFIRM_SETTING_TIME ) ||
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
  sleepservice.notifyWatchFaceActivity();

  accel.resetTaps();
  (void)accel.getSingleTap();
  (void)accel.getDoubleTap();

  while (tof.getGesture() != GESTURE_NONE)
  {
    delay(1);
  }
}

void WatchFaceMain::startup()
{
  panel = MAIN;
  needssetup = true;
  blinking = false;
  textmessageservice.deactivate();
  resetOrientationGestureState();
  sleepservice.notifyWatchFaceActivity();
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
    resetOrientationGestureState();
    captureUprightOrientation();
    captureTofUprightOrientation();

    Serial.println(F("MAIN: flushing TOF after wake/main entry"));
    uint32_t tofFlushUntil = millis() + 1500;
    while (millis() < tofFlushUntil)
    {
      (void)tof.getGesture();
      delay(20);
    }
    Serial.println(F("MAIN: TOF flush complete"));
    return;
  }

  // MAIN: Orientation-driven twist behavior.
  // - TOF gestures are only honored while nearly upright.
  // - Leaving upright arms the gesture.
  // - Returning to upright before Set Time hold completes shows Digital Time.
  // - Holding more than 90 degrees away from upright for 2 seconds enters Set Time.
  if ( millis() - mainwaiter > 2000 )
  {
    if ( ! g_uprightCaptured ) captureUprightOrientation();

    float dot = currentDotFromUpright();
    bool nearlyUpright = dot >= ORIENTATION_UPRIGHT_DOT_THRESHOLD;
    bool awayEnoughToArm = dot <= ORIENTATION_ARM_DOT_THRESHOLD;
    bool past90 = dot <= ORIENTATION_SETTIME_DOT_THRESHOLD;

    if ( ! g_twistHoldArmed )
    {
      if ( awayEnoughToArm )
      {
        Serial.printf( "MAIN: twist/orientation armed dot=%0.2f\n", dot );
        g_twistHoldArmed = true;
        g_twistHoldStartedAt = millis();
        g_setTimePast90StartedAt = 0;
        g_ignoreTofUntil = millis() + 4000;
        noMovementTime = millis();
      }
    }
    else
    {
      // The user rotated away and then returned upright: normal Digital Time.
      if ( nearlyUpright )
      {
        Serial.println( F("MAIN: returned upright -> Digital Time") );
        resetOrientationGestureState();
        g_ignoreTofUntil = millis() + 2000;
        changeTo( DISPLAYING_TIME, true, WatchFaceFlip1_video );
        return;
      }

      // The user is holding the device more than 90 degrees from upright.
      if ( past90 )
      {
        if ( g_setTimePast90StartedAt == 0 )
        {
          Serial.printf( "MAIN: past 90 degrees, Set Time hold started dot=%0.2f\n", dot );
          g_setTimePast90StartedAt = millis();
        }

        if ( millis() - g_setTimePast90StartedAt >= ORIENTATION_SETTIME_HOLD_MS )
        {
          Serial.println( F("MAIN: held past 90 degrees -> Set Time") );

          haptic.playEffect( 66 );   // strong/long buzz: Set Time accepted

          resetOrientationGestureState();
          g_ignoreTofUntil = millis() + 3000;
          g_setTimeProtected = true;

          textmessageservice.stop();
          drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
          panel = SETTING_TIME;
          needssetup = true;
          textmessageservice.stop();
          noMovementTime = millis();
          sleepservice.notifyWatchFaceActivity();

          return;
        }
      }
      else
      {
        // Still away from upright, but not past 90 degrees. Keep armed, but do
        // not count toward Set Time until it is past 90 again.
        g_setTimePast90StartedAt = 0;
      }
    }
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

  // Time setting now uses REST hold, not knocks/taps.
  (void)accel.getDoubleTap();

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
  if ( needssetup )
  {
    Serial.println( F("SETTING_TIME: HOUR") );
    needssetup = false;
    noMovementTime = millis();

    // Clear stale gesture state so previous taps/tilts cannot alter time.
    accel.resetTaps();
    (void)accel.getSingleTap();
    (void)accel.getDoubleTap();
    resetSetTimeTiltGate();

    hour = realtimeclock.getHour();
    minute = realtimeclock.getMinute();
    if ( hour < 1 || hour > 12 ) hour = 12;
    if ( minute < 0 || minute > 59 ) minute = 0;

    g_confirmHour = hour;
    g_confirmMin  = minute;

    drawSetTimePanel( F("Hours"), hour, minute, F(""), F("") );
    haptic.playEffect( 4 );
    return;
  }

  sleepservice.notifyWatchFaceActivity();

  if ( updateTimeLeft() )
  {
    g_confirmHour = hour;
    Serial.print( F("SETTING_TIME: hour accepted by hourglass = ") );
    Serial.println( g_confirmHour );
    haptic.playEffect( 4 );
    changeTo( SETTING_MINUTE, true, "none" );
    return;
  }

  int step = getSetTimeTiltStep(SETTIME_HOUR_REPEAT_MS);
  if ( step != 0 )
  {
    hour += step;
    if ( hour > 12 ) hour = 1;
    if ( hour < 1 ) hour = 12;

    g_confirmHour = hour;
    noMovementTime = millis();
    drawSetTimePanel( F("Hours"), hour, minute, F(""), F("") );
    haptic.playEffect( 14 );

    Serial.print( F("Hour = ") );
    Serial.println( hour );
  }
}

void WatchFaceMain::settingminute()
{
  if ( needssetup )
  {
    Serial.println( F("SETTING_TIME: MINUTE") );
    needssetup = false;
    noMovementTime = millis();

    accel.resetTaps();
    (void)accel.getSingleTap();
    (void)accel.getDoubleTap();
    resetSetTimeTiltGate();

    drawSetTimePanel( F("Minutes"), hour, minute, F(""), F("") );
    haptic.playEffect( 14 );
    return;
  }

  sleepservice.notifyWatchFaceActivity();

  if ( updateTimeLeft() )
  {
    g_confirmHour = hour;
    g_confirmMin  = minute;

    Serial.printf("Accepted new time setting: %d:%02d\n", g_confirmHour, g_confirmMin);
    realtimeclock.setHourMinute(g_confirmHour, g_confirmMin);
    realtimeclock.saveClockToNVS();

    realtimeclock.setHourMinute(g_confirmHour, g_confirmMin);
    realtimeclock.saveClockToNVS();

    clearSetTimeExitNoise();
    
    noMovementTime = millis();
    sleepservice.notifyWatchFaceActivity();

    haptic.playEffect( 76 );

    changeTo( MAIN, true, WatchFaceFlip3_video );
    return;
  }

  int step = getSetTimeTiltStep(SETTIME_REPEAT_MS);
  if ( step != 0 )
  {
    minute += step;
    if ( minute > 59 ) minute = 0;
    if ( minute < 0 ) minute = 59;

    g_confirmMin = minute;
    noMovementTime = millis();
    drawSetTimePanel( F("Minutes"), hour, minute, F(""), F("") );
    haptic.playEffect( 14 );

    Serial.print( F("Minute = ") );
    Serial.println( minute );
  }
}

void WatchFaceMain::drawHourMinute( int hourc, int minutec )
{
  drawSetTimePanel( F("Set Time"), hourc, minutec, F(""), F("") );
}

void WatchFaceMain::drawSetTimePanel( String title, int hourc, int minutec, String help1, String help2 )
{
  // Show minutes/hours
  String mef = String( hourc );
  mef += F(":");
  if ( minutec < 10 ) mef += F("0");
  mef += String( minutec );

  Serial.print( "drawSetTimePanel " );
  Serial.print( mef );
  Serial.println( "." );

  drawImageFromFile( wfMain_SetTime_Background_Shortie, true, 0, 0 );
  textmessageservice.updateTempTime( mef );

  // Mode message: Hours or Minutes
  gfx->setFont(&ScienceFair14pt7b);
  gfx->setTextSize(0);
  gfx->setTextColor( COLOR_PANTONE_102 );
  gfx->setCursor( 50, 105 );
  gfx->println( title );

  digitalWrite(Display_SPI_BK, LOW);
}

bool WatchFaceMain::isTwistSetTimeArmed()
{
  return g_twistHoldArmed;
}

static void clearSetTimeExitNoise()
{
  accel.resetTaps();

  (void)accel.getSingleTap();
  (void)accel.getDoubleTap();

  resetSetTimeTiltGate();

  resetOrientationGestureState();

  textmessageservice.stop();
  textmessageservice.deactivate();

  while (tof.getGesture() != GESTURE_NONE)
  {
    delay(1);
  }
}

void WatchFaceMain::confirmsettingtime()
{
  // The new Blaine flow no longer uses a separate confirm panel. If an old
  // transition ever reaches here, save and return to main safely.
  if ( needssetup )
  {
    Serial.println( F("CONFIRM_SETTING_TIME: legacy fallback") );
    textmessageservice.stop();
    textmessageservice.deactivate();
    needssetup = false;
  }

  realtimeclock.setHourMinute(g_confirmHour, g_confirmMin);
  realtimeclock.saveClockToNVS();
  changeTo( MAIN, true, WatchFaceFlip3_video );
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