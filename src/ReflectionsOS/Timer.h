/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _TIMER_  
#define _TIMER_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

class Timer
{
  public:
    Timer();
    void begin();
    void loop();

    void setTime( int minutes ); 
    int getTime();
    void start();
    void stop();
    bool status();

  private:
    unsigned long timertimer;
    int timeleft;
    bool running;
};

#endif // _TIMER_
