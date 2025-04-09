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

  catFaceIndex = 1;
  catFaceTimer = millis();
  catFaceDirection = true;
  catFaceWait = rand() % 30000;

  tilttimer = millis();
  currentNeutralMeasurementY = accel.getYreading() + 15000;
  currentNeutralMeasurementX = accel.getXreading() + 15000;

  battimer = millis();
  batcount = 0;
  batlev = 0;

  oldHour = 0;
  oldMinute = 0;
  oldBattery = 5;
  oldBlink = 0;

  gpsflag = false;
  gpsx = 0;
  gpsy = 0;

  referenceY = accel.getYreading() + 15000;   // Current Y-axis tilt value
  waitForNextReference = false;               // Delay reference update after hour change
  lastChangeTime = 0;                         // Timestamp of the last hour change

  referenceX = accel.getXreading() + 15000;   // Current Y-axis tilt value
  waitForNextReferenceX = false;              // Delay reference update after hour change
  lastChangeTimeX = 0;                        // Timestamp of the last hour change

  timertimer = millis();
  notificationflag = false;

  slowman = millis();

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  panel = STARTUP;
  //video.startVideo( WatchFaceOpener_video );
}

/* Returns true when watch face is ok with the processor being put to sleep, to save battery life */

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
  int currentHour2 = realtimeclock.getHour();

  if ( currentHour2 == -1 ) currentHour2 = 2;
  if ( currentHour2 > 12 ) currentHour2 = currentHour2 - 12;
  
  if ( currentHour2 != currentHour )
  {
    currentHour = currentHour2;
    displayUpdateable = true;
    drawitall = true;
  }

  int currentMinute2 = realtimeclock.getMinute();

  if ( currentMinute2 == -1 ) currentMinute2 = 15;

  if ( currentMinute2 != currentMinute )
  {
    currentMinute = currentMinute2;
    displayUpdateable = true;
    drawitall = true;
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
        // Generate a random number between 5000 and 10000
        catFaceDirection = true;       
        catFaceIndex = 1;
        blinking = false;
        catFaceTimer = millis();
        catFaceWait = rand() % 30000;
        drawitall = true;
      }
    }
  }
}

// Draw the elements to the buffer

void WatchFaceMain::showDisplayMain()
{
  if ( ! displayUpdateable ) return;
  displayUpdateable = false;

  if ( drawitall ) start();    // Clear frame buffer

  if ( drawitall ) drawImageFromFile( wfMainBackground, true, 0, 0 );

  String mef;

  // Draw hour image

  if ( ( currentHour != oldHour ) || drawitall )
  {
    oldHour = currentHour;

    if ( currentHour > 20 ) currentHour = 20;

    mef = wfMainHours;
    mef += currentHour;
    mef += wfMainHours2;
    drawImageFromFile( mef, true, 0, 0 );
  } 

  int minute = map( currentMinute, 1, 60, 1, 20) + 1;
  if ( minute > 20 ) minute == 20;

  // Draw minutes image
  if ( ( currentMinute != oldMinute ) || drawitall )
  {
    oldMinute = currentMinute;

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

  if ( ( catFaceIndex != oldBlink ) || drawitall )
  {
    oldBlink = catFaceIndex;

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

  if ( ( millis() - noMovementTime ) > 3000 )
  {
    index = 2;
  }

  if ( ( millis() - noMovementTime ) > 6000 )
  {
    index = 3;
  }

  if ( ( millis() - noMovementTime ) > 9000 )
  {
    index = 4;
  }

  if ( ( millis() - noMovementTime ) > 12000 )
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

void WatchFaceMain::loop()
{
  if ( video.getStatus() )
  {
    return;
  }

  if ( textmessageservice.active() )
  {
    return;
  } 
  
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

  //  Every panel with no change, timeout to the main panel
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
}

void WatchFaceMain::startup()
{
  panel = MAIN;
  needssetup = true;
  maintimer = millis();
  //haptic.playEffect(14);  // 14 Strong Buzz
  blinking = false;
}

void WatchFaceMain::main()
{
  if ( needssetup )
  {
    Serial.println( F("MAIN") );
    needssetup = false;
    accel.tapped();       // Clears any taps
    accel.doubletapped();
    drawitall = true;
    blinking = false;
    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_DIGITAL_TIME;   // DISPLAYING_DIGITAL_TIME;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip1_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( millis() - maintimer > 50 )
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

    int index = random(0, 37);
    textmessageservice.startShow( TextMessageExperiences::DigitalTime, F(""), F("") );
    return;
  }

  if ( accel.doubletapped() )
  {
    panel = SETTING_DIGITAL_TIME;
    needssetup = true;
    textmessageservice.stop();
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_HEALTH_STATISTICS;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    textmessageservice.stop();
    noMovementTime = millis();
    return;
  }

  drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
  textmessageservice.updateTime();
}

void WatchFaceMain::settingdigitaltime()
{
  if ( needssetup )
  {
    Serial.println( F("SETTING_DIGITAL_TIME") );
    needssetup = false;
    noMovementTime = millis();
    tilttimer = millis();
    currentHour = realtimeclock.getHour();
    if ( currentHour > 12 ) currentHour = currentHour - 12;
    currentMinute = realtimeclock.getMinute();
    hourschanging = true;

    currentNeutralMeasurementY = accel.getYreading() + 15000; // Current Y-axis tilt value
    currentNeutralMeasurementX = accel.getXreading() + 15000; // Current Y-axis tilt value

    drawImageFromFile( wfMain_SetTime_Background_Hour, true, 0, 0 );
    drawHourMinute( currentHour, currentMinute, hourschanging );
    return;
  }

  if ( accel.tapped() )
  {
    panel = MAIN;
    video.startVideo( WatchFaceFlip3_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    drawitall = true;
    return;
  }

  if ( updateTimeLeft() )
  {
    realtimeclock.setTime( currentHour, currentMinute, 0 );    // Set new time
    panel = MAIN;
    video.startVideo( WatchFaceFlip3_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    drawitall = true;
    return;
  }

  // Change time by tilting left-right for minutes and up-down for hours

  if ( millis() - tilttimer > 500 )
  {
    tilttimer = millis();

    int measurementY = accel.getYreading() + 15000;

    // Calculate Neutral Zone boundaries based on the currentNeutralMeasurement
    int lowerBoundY = currentNeutralMeasurementY - ( currentNeutralMeasurementY * .10 );
    int upperBoundY = currentNeutralMeasurementY + ( currentNeutralMeasurementY * .10 );

    // Neutral Zone: No change in Time value
    if ( ! ( ( measurementY >= lowerBoundY ) && ( measurementY <= upperBoundY ) ) )
    {
      // Increase Zone: Below the Neutral Zone
      if ( measurementY < lowerBoundY ) 
      {
        currentHour = (currentHour == 12) ? 1 : currentHour + 1;
        //currentNeutralMeasurementY = currentNeutralMeasurementY - 100;
        hourschanging = true;
        noMovementTime = millis();
        drawHourMinute( currentHour, currentMinute, hourschanging );
        return;
      }
      
      // Decrease Zone: Above the Neutral Zone
      if ( measurementY > upperBoundY ) 
      {
        currentHour = (currentHour == 1) ? 12 : currentHour - 1; 
        //currentNeutralMeasurementY = currentNeutralMeasurementY + 100;
        hourschanging = true;
        noMovementTime = millis();
        drawHourMinute( currentHour, currentMinute, hourschanging );
        return;
      }
    }

    int measurementX = accel.getXreading() + 15000;

    // Calculate Neutral Zone boundaries based on the currentNeutralMeasurement
    int lowerBoundX = currentNeutralMeasurementX - (currentNeutralMeasurementX * .10);
    int upperBoundX = currentNeutralMeasurementX + (currentNeutralMeasurementX * .10);
    
    // Neutral Zone: No change in Time value
    if ( ! ( ( measurementX >= lowerBoundX ) && ( measurementX <= upperBoundX ) ) )
    {
      // Increase Zone: Below the Neutral Zone
      if ( measurementX < lowerBoundX ) 
      {
        currentMinute++;
        if ( currentMinute > 59 ) currentMinute = 1;

        hourschanging = false;
        noMovementTime = millis();
        drawHourMinute( currentHour, currentMinute, hourschanging );
        return;
      }
      
      // Decrease Zone: Above the Neutral Zone
      if ( measurementX > upperBoundX ) 
      {
        currentMinute--;
        if ( currentMinute == 0 ) currentMinute = 59;

        //currentNeutralMeasurementX = currentNeutralMeasurementX + 50;
        hourschanging = false;
        noMovementTime = millis();
        drawHourMinute( currentHour, currentMinute, hourschanging );
        return;
      }
    }
  }
}

void WatchFaceMain::drawHourMinute( int currentHour, int currentMinute, bool hourschanging )
{
  String mef = String( currentHour );
  mef += F(":");
  if ( currentMinute < 10 )
  {
    mef += F("0");
  }
  mef += String( currentMinute );

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

  if ( accel.tapped() )
  {
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    realtimeclock.setTime( currentHour, currentMinute, 0 );
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

  drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
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

  if ( accel.tapped() )
  {
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    noMovementTime = millis();
    needssetup = true;
    steps.resetStepCount();
    return;
  }

  drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
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
    textmessageservice.stop();
    return;
  }

  if ( accel.tapped() && ( noMovementTime > 5000 ) )
  {
    panel = MAIN;
    //haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    timerservice.start();
    return;
  }

  drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
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
    return;
  }

  if ( accel.doubletapped() )
  {
    panel = CONFIRM_CLEAR_STEPS;
    needssetup = true;
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_TIMER;
    video.startVideo( WatchFaceFlip2_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
  textmessageservice.updateHealth( steps.howManySteps() );
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

  if ( accel.doubletapped() )
  {
    panel = SETTING_TIMER;
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    referenceY = accel.getYreading() + 15000; // Current Y-axis tilt value
    return;
  }

  if ( accel.tapped() )
  {
    needssetup = true;
    panel = MAIN;
    drawitall = true;
    video.startVideo( WatchFaceFlip3_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    noMovementTime = millis();
    return;
  }

  drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
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
    currentHour = timerservice.getTime();
    tilttimer = millis();
    waitForNextReference = false;
    return;
  }

  if ( updateTimeLeft() )
  {
    needssetup = true;
    panel = CONFIRM_START_TIMER;
    //video.startVideo( WatchFaceFlip2_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    noMovementTime = millis();
    textmessageservice.stop();
    timerservice.start();
    return;
  }

  /*
  if ( accel.tapped() && ( noMovementTime > 5000 ) )
  {
    panel = MAIN;
    needssetup = true;
    video.startVideo( WatchFaceFlip3_video );
    //haptic.playEffect(14);  // 14 Strong Buzz
    textmessageservice.stop();
    return;
  }
  */

  if ( millis() - tilttimer > 500 )
  {
    tilttimer = millis();

    int measurementY = accel.getYreading() + 15000;

    // Calculate Neutral Zone boundaries based on the currentNeutralMeasurement
    int lowerBoundY = currentNeutralMeasurementY - ( currentNeutralMeasurementY * .10 );
    int upperBoundY = currentNeutralMeasurementY + ( currentNeutralMeasurementY * .10 );
    
    // Neutral Zone: No change in Time value
    if ( ! ( ( measurementY >= lowerBoundY ) && ( measurementY <= upperBoundY ) ) )
    {
      // Increase Zone: Below the Neutral Zone
      if ( measurementY < lowerBoundY ) 
      {
        currentHour = (currentHour == 60) ? 1 : currentHour + 1;
        currentNeutralMeasurementY = currentNeutralMeasurementY - 50;
        hourschanging = true;
        noMovementTime = millis();
      }
      
      // Decrease Zone: Above the Neutral Zone
      if ( measurementY > upperBoundY ) 
      {
        currentHour = (currentHour == 1) ? 60 : currentHour - 1; 
        currentNeutralMeasurementY = currentNeutralMeasurementY + 50;
        hourschanging = true;
        noMovementTime = millis();
      }
    }

    String met = String( currentHour );
    drawImageFromFile( wfMain_SetTimer_Background_Shortie, true, 0, 0 );
    textmessageservice.updateTempTime( met );
  }
}
