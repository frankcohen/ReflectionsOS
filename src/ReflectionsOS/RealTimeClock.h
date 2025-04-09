/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _RealTimeClock_
#define _RealTimeClock_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "time.h"
#include "gps.h"

#include <Preferences.h>

extern GPS gps;

class RealTimeClock
{
  public:
    RealTimeClock();
    void begin();
    void loop();

    int getHour();
    int getMinute();
    
    void setTime( int hour, int minute, int ampm );

  private:
    Preferences preferences;
    
};

#endif // _RealTimeClock_

