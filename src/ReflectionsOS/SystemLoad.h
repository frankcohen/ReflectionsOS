/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef SYSTEMLOAD_H
#define SYSTEMLOAD_H

#include "RealTimeClock.h"
extern RealTimeClock realtimeclock;

#include <Arduino.h>

#include "config.h"
#include "secrets.h"

class SystemLoad {

public:
  SystemLoad();
  void begin();
  void loop();
  void printStats();
  void printHeapSpace( String messsage );

  void logtasktime( unsigned long tsktime, int mes, String mname );

private:
  unsigned long previousMillis;
  unsigned long interval;

  unsigned long accumulatedTaskTime;

  unsigned long accumulatedTaskTime1;
  String taskName1;
  unsigned long accumulatedTaskTime2;
  String taskName2;
  unsigned long accumulatedTaskTime3;
  String taskName3;

  unsigned long oldTime;
};

#endif // SYSTEMLOAD_H

