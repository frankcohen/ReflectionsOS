/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for audio tasks.

*/

#ifndef _audio_
#define _audio_

#include "Arduino.h"
#include "AudioPlayer.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

class AudioPlayer
{
  public:
    AudioPlayer();
    void begin();
    void loop();
    void start( String filename );

  private:
    Audio _audio;
};

#endif // _audio_
