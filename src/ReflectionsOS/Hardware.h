/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _HARDWARE_
#define _HARDWARE_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "SD.h"
#include "SPI.h"
#include "Wire.h"
#include "Logger.h"

class Hardware
{
  public:
    Hardware();
    void begin();
    void loop();
    bool getMounted();

    void powerDownComponents();
    void powerUpComponents();

  private:
    bool NANDMounted;

};

#endif // _HARDWARE_
