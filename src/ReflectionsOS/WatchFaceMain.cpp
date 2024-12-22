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

  battimer = millis();
  batcount = 0;
  batlev = 0;

  oldHour = 0;
  oldMinute = 0;
  oldBattery = 5;
  oldBlink = 0;

  referenceY = accel.getYreading() + 15000;   // Current Y-axis tilt value
  waitForNextReference = false;               // Delay reference update after hour change
  lastChangeTime = 0;                         // Timestamp of the last hour change

  referenceX = accel.getXreading() + 15000;   // Current Y-axis tilt value
  waitForNextReferenceX = false;              // Delay reference update after hour change
  lastChangeTimeX = 0;                        // Timestamp of the last hour change

  timertimer = millis();
  notificationflag = false;

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  panel = STARTUP;
  video.startVideo( WatchFaceOpener_video );
}

// Timer notice indicator

void WatchFaceMain::updateTimerNotice()
{
  if ( timerservice.status() )
  {
    notificationflag = true;
  }
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

  show();

  drawitall = false;
}

// Draws hour glass time left display, 3, 2, 1
// Returns true when time is up

bool WatchFaceMain::updateTimeLeft()
{
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

  if ( experienceservice.active() )
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
  if ( ( ( millis() - noMovementTime ) > 17000 ) && ( panel != MAIN ) )
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
  haptic.playEffect(14);  // 14 Strong Buzz
  blinking = false;
}

void WatchFaceMain::main()
{
  if ( needssetup )
  {
    Serial.println( "MAIN" );
    needssetup = false;
    blinking = false;
    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_DIGITAL_TIME;   // DISPLAYING_DIGITAL_TIME;
    haptic.playEffect(14);  // 14 Strong Buzz
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
    Serial.println( "DISPLAYING_DIGITAL_TIME" );
    needssetup = false;
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = SETTING_DIGITAL_TIME;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    noMovementTime = millis();
    referenceY = accel.getYreading() + 15000; // Current Y-axis tilt value
    referenceX = accel.getXreading() + 15000; // Current Y-axis tilt value
    return;
  }
  
  drawImageFromFile( wfMain_Time_Background, true, 0, 0 );
  textmessageservice.updateTime();
}

void WatchFaceMain::settingdigitaltime()
{
  if ( needssetup )
  {
    Serial.println( "SETTING_DIGITAL_TIME" );
    needssetup = false;
    noMovementTime = millis();
    currentHour = realtimeclock.getHour();
    if ( currentHour > 12 ) currentHour = currentHour - 12;
    currentMinute = realtimeclock.getMinute();

    hourschanging = true;
    drawImageFromFile( wfMain_SetTime_Background_Hour, true, 0, 0 );

    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_HEALTH_STATISTICS;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = CONFIRM_TIME;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    textmessageservice.stop();
    needssetup = true;
    return;
  }

  // Change time by tilting left-right and up-down

  if ( millis() - tilttimer > 500 )
  {
    tilttimer = millis();

    rawY = accel.getYreading() + 15000; // Current Y-axis tilt value
    threshold = referenceY * 0.1;

    rawX = accel.getXreading() + 15000; // Current Y-axis tilt value
    thresholdX = referenceX * 0.1;

    const unsigned long repeatDelay = 500; // Auto-repeat delay in milliseconds

    if ( waitForNextReference ) 
    {
      if (millis() - lastChangeTime >= 1000) 
      { // Wait 1 second before updating reference
        referenceY = rawY; // Update reference to new baseline
        waitForNextReference = false;
        Serial.print("Reference Y updated to: ");
        Serial.println(referenceY);
      }
    }

    if ( waitForNextReferenceX ) 
    {
      if (millis() - lastChangeTimeX >= 1000) 
      { // Wait 1 second before updating reference
        referenceX = rawX; // Update reference to new baseline
        waitForNextReferenceX = false;
        Serial.print("Reference X updated to: ");
        Serial.println(referenceX);
      }
    }

    if (rawY > referenceY + threshold) 
    {
      currentHour++;
      if (currentHour > 12) 
      {
        currentHour = 1; // Wrap around to 1 after 12
      }
      Serial.print("Hour increased to: ");
      Serial.println(currentHour);

      lastChangeTime = millis();
      lastRepeatTime = millis();
      waitForNextReference = true; // Start delay before updating reference
      hourschanging = true;
      noMovementTime = millis();
    }

    else if (rawY < referenceY - threshold) 
    {
      currentHour--;
      if (currentHour < 1) 
      {
        currentHour = 12; // Wrap around to 12 after 1
      }

      Serial.print("Hour decreased to: ");
      Serial.println(currentHour);

      lastChangeTime = millis();
      lastRepeatTime = millis();
      waitForNextReference = true; // Start delay before updating reference
      hourschanging = true;
      noMovementTime = millis();
    }

    // Auto-repeat check after 1 second
    else if ( millis() - lastRepeatTime >= 500 ) 
    {
      if (rawY >= referenceY + threshold) 
      {
        currentHour++;
        if (currentHour > 12) 
        {
          currentHour = 1; // Wrap around to 1 after 12
        }
        Serial.print("Auto-repeat: Hour increased to: ");
        Serial.println(currentHour);
        lastRepeatTime = millis(); // Update repeat timestamp
        hourschanging = true;
        noMovementTime = millis();
      } 

      else if (rawY <= referenceY - threshold) 
      {
        currentHour--;
        if (currentHour < 1) 
        {
          currentHour = 12; // Wrap around to 12 after 1
        }

        Serial.print("Auto-repeat: Hour decreased to: ");
        Serial.println(currentHour);
        hourschanging = true;
        lastRepeatTime = millis(); // Update repeat timestamp
        noMovementTime = millis();
      }
    }

    if (rawX > referenceX + thresholdX) 
    {
      currentMinute++;
      if (currentMinute > 59) 
      {
        currentMinute = 1; // Wrap around to 1 after 59
      }
      Serial.print("Minute increased to: ");
      Serial.println(currentMinute);

      lastChangeTimeX = millis();
      lastRepeatTimeX = millis();
      waitForNextReferenceX = true; // Start delay before updating reference
      hourschanging = false;
      noMovementTime = millis();
    }

    else if (rawX < referenceX - thresholdX) 
    {
      hourschanging = false;
      currentMinute--;
      if (currentMinute < 1) 
      {
        currentMinute = 59; // Wrap around to 59 after 1
      }

      Serial.print("Minute decreased to: ");
      Serial.println(currentMinute);

      lastChangeTimeX = millis();
      lastRepeatTimeX = millis();
      waitForNextReferenceX = true; // Start delay before updating reference
      hourschanging = false;
      noMovementTime = millis();
    }

    // Auto-repeat check after 1 second
    else if ( millis() - lastRepeatTimeX >= 500 ) 
    {
      if (rawX >= referenceX + thresholdX) 
      {
        currentMinute++;
        if (currentMinute > 59) 
        {
          currentMinute = 1; // Wrap around to 1 after 59
        }
        Serial.print("Auto-repeat: Minute increased to: ");
        Serial.println(currentMinute);
        lastRepeatTimeX = millis(); // Update repeat timestamp
        hourschanging = false;
        noMovementTime = millis();
      } 

      else if (rawX <= referenceX - thresholdX) 
      {
        currentMinute--;
        if (currentMinute < 1) 
        {
          currentMinute = 24; // Wrap around to 24 after 1
        }

        Serial.print("Auto-repeat: Minute decreased to: ");
        Serial.println(currentMinute);
        lastRepeatTimeX = millis(); // Update repeat timestamp
        noMovementTime = millis();
        hourschanging = false;
      }
    }

    String mef = String( currentHour );
    mef += ":";
    if ( currentMinute < 10 )
    {
      mef += "0";
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
}

void WatchFaceMain::changetime()
{
  if ( needssetup )
  {
    Serial.println( "CONFIRM_TIME" );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_HEALTH_STATISTICS;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = MAIN;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    realtimeclock.setTime( currentHour, currentMinute, 0 );
    return;
  }

  drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
  textmessageservice.drawCenteredMesssage( "Set Time?" );

}

void WatchFaceMain::clearsteps()
{
  if ( needssetup )
  {
    Serial.println( "CONFIRM_CLEAR_STEPS" );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = DISPLAYING_TIMER;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip2_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = MAIN;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    steps.resetStepCount();
    return;
  }

  drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
  textmessageservice.drawCenteredMesssage( "Clear Steps?" );

}

void WatchFaceMain::starttimer()
{
  if ( needssetup )
  {
    Serial.println( "CONFIRM_START_TIMER" );
    needssetup = false;
    drawImageFromFile( wfMain_Face_BlueDot_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = MAIN;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    needssetup = true;
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = MAIN;
    haptic.playEffect(14);  // 14 Strong Buzz
    video.startVideo( WatchFaceFlip3_video );
    textmessageservice.stop();
    needssetup = true;
    timerservice.start();
    return;
  }

  drawImageFromFile( wfMain_Time_Background_Shortie, true, 0, 0 );
  textmessageservice.drawCenteredMesssage( "Start Timer?" );
}

void WatchFaceMain::displayinghealthstatistics()
{
  if ( needssetup )
  {
    Serial.println( "DISPLAYING_HEALTH_STATISTICS" );
    needssetup = false;
    drawImageFromFile( wfMain_Health_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    panel = CONFIRM_CLEAR_STEPS;
    video.startVideo( WatchFaceFlip2_video );
    haptic.playEffect(14);  // 14 Strong Buzz
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
    Serial.println( "DISPLAYING_TIMER" );
    needssetup = false;
    drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
    noMovementTime = millis();
    return;
  }

  if ( accel.tapped() )
  {
    needssetup = true;
    panel = SETTING_TIMER;
    video.startVideo( WatchFaceFlip2_video );
    haptic.playEffect(14);  // 14 Strong Buzz
    noMovementTime = millis();
    textmessageservice.stop();
    referenceY = accel.getYreading() + 15000; // Current Y-axis tilt value
    return;
  }

  drawImageFromFile( wfMain_Timer_Background, true, 0, 0 );
  textmessageservice.updateTimer( timerservice.getTime() );
}

void WatchFaceMain::settingtimer()
{
  if ( needssetup )
  {
    Serial.println( "SETTING_TIMER" );
    needssetup = false;
    drawImageFromFile( wfMain_SetTimer_Background, true, 0, 0 );
    noMovementTime = millis();
    currentHour = 20;
    tilttimer = millis();
    waitForNextReference = false;
    return;
  }

  if ( accel.tapped() )
  {
    needssetup = true;
    panel = CONFIRM_START_TIMER;
    video.startVideo( WatchFaceFlip2_video );
    haptic.playEffect(14);  // 14 Strong Buzz
    noMovementTime = millis();
    textmessageservice.stop();
    return;
  }

  if ( updateTimeLeft() )
  {
    panel = MAIN;
    needssetup = true;
    video.startVideo( WatchFaceFlip3_video );
    haptic.playEffect(14);  // 14 Strong Buzz
    textmessageservice.stop();
    timerservice.start();
    return;
  }

  String met = String( currentHour );
  drawImageFromFile( wfMain_SetTimer_Background_Shortie, true, 0, 0 );
  textmessageservice.updateTempTime( met );

  if ( millis() - tilttimer > 500 )
  {
    tilttimer = millis();

    rawY = accel.getYreading() + 15000; // Current Y-axis tilt value
    threshold = referenceY * 0.1;

    const unsigned long repeatDelay = 500; // Auto-repeat delay in milliseconds

    if ( waitForNextReference ) 
    {
      if (millis() - lastChangeTime >= 1000) 
      { // Wait 1 second before updating reference
        referenceY = rawY; // Update reference to new baseline
        waitForNextReference = false;
        Serial.print("Timer reference Y updated to: ");
        Serial.println(referenceY);
      }
    }

    if (rawY > referenceY + threshold) 
    {
      currentHour++;
      if (currentHour > 30) 
      {
        currentHour = 1; // Wrap around to 1 after 12
      }
      Serial.print("Timer increased to: ");
      Serial.println(currentHour);

      lastChangeTime = millis();
      lastRepeatTime = millis();
      waitForNextReference = true; // Start delay before updating reference
      hourschanging = true;
      noMovementTime = millis();
    }

    else if (rawY < referenceY - threshold) 
    {
      currentHour--;
      if (currentHour < 1) 
      {
        currentHour = 30; // Wrap around to 12 after 1
      }

      Serial.print("Timer decreased to: ");
      Serial.println(currentHour);

      lastChangeTime = millis();
      lastRepeatTime = millis();
      waitForNextReference = true; // Start delay before updating reference
      hourschanging = true;
      noMovementTime = millis();
    }

    // Auto-repeat check after 1 second
    else if ( millis() - lastRepeatTime >= 500 ) 
    {
      if (rawY >= referenceY + threshold) 
      {
        currentHour++;
        if (currentHour > 30) 
        {
          currentHour = 1; // Wrap around to 1 after 12
        }
        Serial.print("Auto-repeat: Timer increased to: ");
        Serial.println(currentHour);
        lastRepeatTime = millis(); // Update repeat timestamp
        hourschanging = true;
        noMovementTime = millis();
      }

      else if (rawY <= referenceY - threshold) 
      {
        currentHour--;
        if (currentHour < 1) 
        {
          currentHour = 30; // Wrap around to 12 after 1
        }

        Serial.print("Auto-repeat: Timer decreased to: ");
        Serial.println(currentHour);
        hourschanging = true;
        lastRepeatTime = millis(); // Update repeat timestamp
        noMovementTime = millis();
      }
    }
  }
}
