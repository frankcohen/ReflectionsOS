#ifndef _AUDIO_
#define _AUDIO_

#include "config.h"
#include "secrets.h"

#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioSampleAAC.h"
#include <SD.h>

// Depends on https://github.com/earlephilhower/ESP8266Audio

class Audio
{
  public:
    Audio();
    void begin();
    void loop();
    bool test();

  private:
    AudioFileSourcePROGMEM *in;
    AudioGeneratorAAC *aac;
    AudioOutputI2S *out;

};

#endif // _AUDIO_
