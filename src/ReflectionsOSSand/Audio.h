/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _AUDIO_
#define _AUDIO_

#include "config.h"
#include "secrets.h"

#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <SD.h>

// Depends on https://github.com/earlephilhower/ESP8266Audio

class Audio
{
  public:
    Audio();
    void begin();
    void loop();
    bool test();
    void play( String aname );

  private:
    AudioFileSourceSD *in;
    AudioGeneratorMP3 *mp3;
    AudioOutputI2S *out;
    //AudioOutputI2S *out;
};

#endif // _AUDIO_
