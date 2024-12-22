/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _TIMERSERVICE_  
#define _TIMERSERVICE_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

class TimerService
{
  public:
    TimerService();
    void begin();
    void loop();

    void setTime( int minutes ); 
    int getTime();
    void start();
    void stop();
    bool status();

  private:
    unsigned long timertimer;
    unsigned long alarmduration;
    int timeleft;
    bool running;
    bool alarmnotice;
};

#endif // TimerService
