/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the video display.

Notes:

Using Arduino_GFX class by @moononournation
https://github.com/moononournation/Arduino_GFX
For speed and support of MPEG video

Thank you Pawel A. Hernik (https://youtu.be/o3AqITHf0mo) for tips on how to stream
raw/uncompressed video to the display.

Thank you Earle F. Philhower, III, earlephilhower@yahoo.com, for the WAV audio
player library at https://github.com/earlephilhower/ESP8266Audio

Note: To play 240x240 MJPEG uncompressed files requires the audio at
 a sample rate of 8000 kHz, 16 bits, stereo format, WAVE format

*/

#ifndef _Wifi_
#define _Wifi_

#include "config.h"
#include <WiFiUdp.h>
#include "secrets.h"
#include <WiFiMulti.h>

#define MJPEG_FILENAME ".mjpeg"

class Wifi
{
  public:
    Wifi();
    void begin();
    void loop();

  private:
    WiFiMulti _wifiMulti;
};

#endif // _Wifi_
