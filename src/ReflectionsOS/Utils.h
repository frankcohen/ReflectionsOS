/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _UTILS_
#define _UTILS_

#include <SPI.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "config.h"
#include "secrets.h"

#include "Wire.h"

#include "WiFi.h"
#include "WiFiMulti.h"

class Utils
{
  public:
    Utils();
    void begin();
    void loop();
    void WireScan();
    void hexDump( File file );

  private:
    WiFiMulti _wifiMulti;
    long _connectionTimer;
};

#endif // _UTILS_
