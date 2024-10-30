/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _OTA_
#define _OTA_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"

#include "esp_ota_ops.h"        //included header file for esp_ota driver
#include "FS.h"
#include "SD.h"

#include <WiFiUdp.h>
#include <WiFiMulti.h>

class OTA
{
  public:
    OTA();
    void begin();
    bool update();
    void loop();

  private:

};

#endif // _OTA_
