/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/earlephilhower/ESP8266Audio
*/

#include "Audio.h"

Audio::Audio(){}

void Audio::begin()
{ 
}

bool Audio::test()
{
  audioLogger = &Serial;
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S();
  out -> SetPinout( I2S_bclkPinP, I2S_wclkPinP, I2S_doutPinP);
  //Serial.println( F( "Playing audio sample" ) );
  #define I2S_DOUT      27
#define I2S_BCLK      26
#define I2S_LRC       25

  return true;
}

void Audio::loop()
{
  if ( aac->isRunning() )
  {
    aac->loop();
  }
  else
  {
    aac -> stop();
    //Serial.println( F( "Audio done\n" ) );
  }
}
