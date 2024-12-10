/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _GPS_
#define _GPS_

#include "config.h"
#include "secrets.h"

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include "time.h"

#define GPSBaud 9600

class GPS
{
  public:
    GPS();
    void begin();
    void loop();
    bool test();

    void printHeader();
    void printLocation();
    void printFloat(float val, bool valid, int len, int prec);
    void printInt(unsigned long val, bool valid, int len);
    void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
    void printStr(const char *str, int len);

    unsigned int getProcessed();
    
    bool isActive();

    bool isTimeValid();
    bool isDateValid();

    unsigned int getMonth();
    unsigned int getDay();
    unsigned int getYear();
    unsigned int getHour();
    unsigned int getMinute();
    unsigned int getSecond();

    void on();
    void off();

  private:
    TinyGPSPlus gps;
    bool active;
    unsigned long gpstime;
};

#endif // _GPS_
