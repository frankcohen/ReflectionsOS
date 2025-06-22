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
  baselineY = accel.getYreading();
  baselineX = accel.getXreading();

  battimer = millis();
  batcount = 0;
  batlev = 0;

  oldHour = 0;
  oldMinute = 0;
  oldBattery = 5;
  oldBlink = 1;

  gpsflag = false;
  gpsx = 0;
  gpsy = 0;

  notificationflag = false;

  slowman = millis();

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  panel = STARTUP;
  video.startVideo( WatchFaceOpener_video );

  textmessageservice.deactivate();

  resetOnces();

  temptimer = millis();

  minuteRedrawtimer = millis();

  _runmode = true;
}

void WatchFaceMain::resetOnces()
{
  onceDigitalTime = true;
  onceHealth = true;
}

/* Returns true when enough time has passed to start another experience */

bool WatchFaceMain::okToExperience()
{
  if ( ( ( panel == STARTUP ) || ( panel == MAIN ) ) && ( millis() - enoughTimePassedtimer > 15000 ) ) return true;
  return false;
}

/* Helps cat to know when it's ok to sleep */

bool WatchFaceMain::okToSleep()
{  
  if ( panel != MAIN ) return false;
  if ( video.getStatus() ) return false;
  if ( textmessageservice.active() ) return false;

  return true;
}

// GPS marker

void WatchFaceMain::updateGPSmarker()
{
  gpsflag = gps.isActive();

  if ( gpsflag )
  {
    float gpsCourse = gps.getCourse(); // Course angle in degrees

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
  if ( ( millis() - battimer ) > 2000 )
  {
    battimer = millis();

    batlev = battery.getBatteryLevel();
    if ( ( batlev != batcount ) && ( batcount < 3 ) ) 
    {
      batcount++;
      displayUpdateable = true;      
    }
  }
}

// Hour and minutes hands to current time

void WatchFaceMain::updateHoursAndMinutes()
{
  int hour2 = realtimeclock.getHour();

  if ( hour2 == -1 ) hour2 = 2;
  if ( hour2 > 12 ) hour2 = hour2 - 12;
  
  if ( hour2 != hour )
  {
    hour = hour2;
    displayUpdateable = true;
    drawFace = true;
  }

  int minute2 = realtimeclock.getMinute();

  if ( minute2 == -1 ) minute2 = 15;

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

    if ( hour > 20 ) hour = 20;

    mef = wfMainHours;
    mef += hour;
    mef += wfMainHours2;
    drawImageFromFile( mef, true, 0, 0 );
  } 

  int minute = map( minute, 1, 60, 1, 20) + 1;
  if ( minute > 20 ) minute == 20;

  // Draw minutes image
  if ( ( minute != oldMinute ) || drawitall )
  {
    oldMinute = minute;

    mef = wfMainMinutes;
    mef += minute ;
    mef += wfMainMinutes2;
    drawImageFromFile( mef, true, 0, 0 );
  }

  // Battery indicator

  if ( ( batcount != oldBattery ) || drawitall )
  {
    oldBattery = batcount;

    mef = wfMainBattery;
    mef += batcount + 1;
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

  if ( ( millis() - noMovementTime ) > ( nomov * 1 ) )
  {
    index = 2;
  }

  if ( ( millis() - noMovementTime ) > ( nomov * 2 ) )
  {
    index = 3;
  }

  if ( ( millis() - noMovementTime ) > ( nomov * 3 ) )
  {
    index = 4;
  }

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
    
    if ( video.getStatus() )
    {
      mef += "1 ";
    }
    else
    {
      mef += "0 ";
    }

    if ( textmessageservice.active() )
    {
      mef += "1 ";
    }
    else
    {
      mef += "0 ";
    }
    
    Serial.println( mef );
  }

  return;
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
    case DISPLAYING_DIGITAL_TIME:
      displayingdigitaltime();
      break;
    case SETTING_DIGITAL_TIME:
      settingdigitaltime();
      break;
    case DISPLAYING_TIMER:
      displayingtimer();
      break;
    case SETTING_TIMER:
      settingtimer();
      break;
    case DISPLAYING_HEALTH_STATISTICS:
      displayinghealthstatistics();
      break;  
    case CONFIRM_TIME:
      changetime();
      break;  
    case CONFIRM_CLEAR_STEPS:
      clearsteps();
      break;  
    case CONFIRM_START_TIMER:
      starttimer();
      break;  
  }

  //  Timeout any panel - except for the main - with no change in 20 seconds

  if ( ( ( millis() - noMovementTime ) > 20000 ) && ( panel != MAIN ) )
  {
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    noMovementTime = millis();
    return;
  }

  if ( millis() - minuteRedrawtimer > 2 * ( 60 * 1000 ) )
  {
    setDrawItAll();
    showDisplayMain();
    minuteRedrawtimer = millis();
  }
}

void WatchFaceMain::startup()
{
  panel = MAIN;
  needssetup = true;
  blinking = false;
  textmessageservice.deactivate();
}

void WatchFaceMain::main()
{
  if ( needssetup )
  {
    Serial.println( F("MAIN") );
    needssetup = false;
    drawitall = true;
    blinking = false;
    resetOnces();
    return;
  }

  if ( accel.getSingleTap() )
  {
    panel = DISPLAYING_DIGITAL_TIME;   // DISPLAYING_DIGITAL_TIME;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip1_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( millis() - maintimer > 50 && ! video.getStatus() )
  {
    maintimer = millis();

    updateBlink();            // Blink eyes
    updateBattery();          // Battery level indicator
    updateHoursAndMinutes();  // Hour and minute hands
    updateTimerNotice();      // Timer notice

    showDisplayMain();
  }
}

void WatchFaceMain::displayingdigitaltime()
{
  if ( needssetup )
  {
    Serial.println( F("DISPLAYING_DIGITAL_TIME") );
    needssetup = false;
    noMovementTime = millis();

    drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
    textmessageservice.startShow( TextMessageExperiences::DigitalTime, "", "" );
    return;
  }

  if ( accel.getDoubleTap() )
  {
    Serial.println( F( "displayingdigitaltime double tapped" ) );

    panel = SETTING_DIGITAL_TIME;
    needssetup = true;
    textmessageservice.stop();
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( accel.getSingleTap() )
  {
    Serial.println( F( "displayingdigitaltime tapped" ) );

    panel = DISPLAYING_HEALTH_STATISTICS;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    textmessageservice.stop();
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( onceDigitalTime )
  {
    onceDigitalTime = false;
    //textmessageservice.updateTime();
  }
}

void WatchFaceMain::setDrawItAll()
{
  drawitall = true;
}

// Check if the current reading is outside the neutral zone
bool WatchFaceMain::isOutsideNeutralZone(float value, float baseline, float neutralFactor) 
{
  return (value < (baseline - baseline * neutralFactor) || value > (baseline + baseline * neutralFactor));
}

// Adjust the baseline by moving it towards the new position slowly (smooth transition)
float WatchFaceMain::adjustBaseline(float baseline, float currentReading, float factor) 
{
  return baseline + factor * (currentReading - baseline);
}

void WatchFaceMain::settingdigitaltime()
{
  if ( needssetup )
  {
    Serial.println( F("SETTING_DIGITAL_TIME") );
    needssetup = false;
    noMovementTime = millis();
    tilttimer = millis();
    hour = realtimeclock.getHour();
    if ( hour > 12 ) hour = hour - 12;
    minute = realtimeclock.getMinute();
    hourschanging = true;

    baselineY = accel.getYreading(); // Current Y-axis tilt value
    baselineX = accel.getXreading(); // Current Y-axis tilt value

    //drawImageFromFile( wfMain_SetTime_Background_Hour, true, 0, 0 );
    drawHourMinute( hour, minute, hourschanging );
    return;
  }

  if ( accel.getSingleTap() )
  {
    panel = MAIN;
    video.startVideo( WatchFaceFlip3_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    needssetup = true;
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    textmessageservice.stop();
    drawitall = true;
    return;
  }

  if ( updateTimeLeft() )
  {
    realtimeclock.setTime( hour, minute, 0 );    // Set new time
    panel = MAIN;
    video.startVideo( WatchFaceFlip3_video );
    needssetup = true;
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    textmessageservice.stop();
    drawitall = true;
    return;
  }

  // Change time by tilting left-right for minutes and up-down for hours

  if ( millis() - tilttimer < tiltspeed ) return;
  
  tilttimer = millis();

  float xFiltered = accel.getXreading();
  float yFiltered = accel.getYreading();

  // Detect horizontal movement (X axis) outside the neutral zone
  if (isOutsideNeutralZone(xFiltered, baselineX, neutralZoneFactor)) {
    if (abs(xFiltered - baselineX) > xThreshold) {
      minute += (xFiltered > baselineX) ? 1 : -1;  // Increase or decrease minute
      if (minute >= 60) minute = 0; // Wrap around if it exceeds 59 minutes
      if (minute < 0) minute = 59; // Wrap around if it goes below 0
      noMovementTime = millis();  // Reset inactivity timer
      hourschanging = false;
      baselineX = adjustBaseline(baselineX, xFiltered, factorLevel);  // Move baseline towards the new X position      
      drawHourMinute( hour, minute, hourschanging );
      return;
    }
  }

  // Detect vertical movement (Y axis) outside the neutral zone
  if (isOutsideNeutralZone(yFiltered, baselineY, neutralZoneFactor)) {
    if (abs(yFiltered - baselineY) > yThreshold) {
      hour += (yFiltered > baselineY) ? 1 : -1;  // Increase or decrease hour
      if (hour > 12) hour = 1; // Wrap around if it exceeds 12 hours
      if (hour < 1) hour = 12; // Wrap around if it goes below 1
      hourschanging = true;
      noMovementTime = millis();  // Reset inactivity timer
      baselineY = adjustBaseline(baselineY, yFiltered, factorLevel);  // Move baseline towards the new Y position
      drawHourMinute( hour, minute, hourschanging );
      return;
    }
  }
}

void WatchFaceMain::drawHourMinute( int hourc, int minutec, bool hourschanging )
{
  String mef = String( hourc );
  mef += F(":");
  if ( minutec < 10 )
  {
    mef += F("0");
  }
  mef += String( minutec );

  if ( hourschanging )
  {
    drawImageFromFile( wfMain_SetTime_Background_Hour_Shortie, true, 0, 0 );
  }
  else
  {
    drawImageFromFile( wfMain_SetTime_Background_Minute_Shortie, true, 0, 0 );
  }
  textmessageservice.updateTempTime( mef );
}

void WatchFaceMain::changetime()
{
  if ( needssetup )
  {
    Serial.println( F("CONFIRM_TIME") );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.getSingleTap() )
  {
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    enoughTimePassedtimer = millis();
    realtimeclock.setTime( hour, minute, 0 );
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = DISPLAYING_HEALTH_STATISTICS;
    //haptic.playEffect(14);  // 14 Strong Buzz
    //video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    noMovementTime = millis();
    return;
  }

  //drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
  textmessageservice.drawCenteredMesssage( F("Tap To Set"), F("New Time") );
}

void WatchFaceMain::clearsteps()
{
  if ( needssetup )
  {
    Serial.println( F("CONFIRM_CLEAR_STEPS") );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = DISPLAYING_TIMER;
    //haptic.playEffect(14);  // 14 Strong Buzz
    //video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( accel.getSingleTap() )
  {
    panel = DISPLAYING_TIMER;
    video.startVideo( WatchFaceFlip2_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    needssetup = true;
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    textmessageservice.stop();
    return;
  }

  //drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
  textmessageservice.drawCenteredMesssage( F("Tap To"), F("Clear Steps") );
}

void WatchFaceMain::starttimer()
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
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    needssetup = true;
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    textmessageservice.stop();
    return;
  }

  if ( accel.getSingleTap() && ( noMovementTime > 5000 ) )
  {
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    enoughTimePassedtimer = millis();
    timerservice.start();
    return;
  }

  //drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
  textmessageservice.drawCenteredMesssage( F("Tap To"), F("Start Timer") );
}

void WatchFaceMain::displayinghealthstatistics()
{
  if ( needssetup )
  {
    Serial.println( F("DISPLAYING_HEALTH_STATISTICS") );
    needssetup = false;
    drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( accel.getDoubleTap() )
  {
    panel = CONFIRM_CLEAR_STEPS;
    needssetup = true;
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( accel.getSingleTap() )
  {
    panel = DISPLAYING_TIMER;
    video.startVideo( WatchFaceFlip2_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    needssetup = true;
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    textmessageservice.stop();
    return;
  }

  if ( onceHealth )
  {
    onceHealth = false;
    drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
    textmessageservice.updateHealth( steps.howManySteps() );
  }
}

void WatchFaceMain::displayingtimer()
{
  if ( needssetup )
  {
    Serial.println( F("DISPLAYING_TIMER") );
    needssetup = false;
    drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( accel.getDoubleTap() )
  {
    panel = SETTING_TIMER;
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( accel.getSingleTap() )
  {
    needssetup = true;
    panel = MAIN;
    drawitall = true;
    video.startVideo( WatchFaceFlip3_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    noMovementTime = millis();
    enoughTimePassedtimer = millis();
    return;
  }

  //drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
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
    enoughTimePassedtimer = millis();
    return;
  }

  if ( updateTimeLeft() )
  {
    needssetup = true;
    panel = CONFIRM_START_TIMER;
    video.startVideo( WatchFaceFlip2_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    noMovementTime = millis();
    textmessageservice.stop();
    timerservice.start();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( accel.getSingleTap() && ( noMovementTime > 5000 ) )
  {
    panel = MAIN;
    needssetup = true;
    video.startVideo( WatchFaceFlip3_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    textmessageservice.stop();
    enoughTimePassedtimer = millis();
    return;
  }

  if ( millis() - tilttimer < 1000 ) return;  
  tilttimer = millis();

  float yFiltered = accel.getYreading();

  // Detect vertical movement (Y axis) outside the neutral zone
  // Note: It says hours here when the timer is really working in minutes, should be fixed later
  if (isOutsideNeutralZone(yFiltered, baselineY, neutralZoneFactor)) {
    if (abs(yFiltered - baselineY) > yThreshold) {
      hour += (yFiltered > baselineY) ? 1 : -1;  // Increase or decrease hour
      if (hour > 30) hour = 1; // Wrap around if it exceeds 30 minutes
      if (hour < 1) hour = 30; // Wrap around if it goes below 1
      noMovementTime = millis();  // Reset inactivity timer
      baselineY = adjustBaseline(baselineY, yFiltered, factorLevel);  // Move baseline towards the new Y position

      String met = String( hour );
      //drawImageFromFile( wfMain_SetTimer_Background_Shortie, true, 0, 0 );
      textmessageservice.updateTempTime( met );

      return;
    }
  }

}
