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
