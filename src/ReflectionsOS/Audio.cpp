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

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
  // Note that the type and string may be in PROGMEM, so copy them to RAM for printf
  char s1[32], s2[64];
  strncpy_P(s1, type, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  strncpy_P(s2, string, sizeof(s2));
  s2[sizeof(s2)-1]=0;
  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
  Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}

void Audio::begin()
{
  /*
  audioLogger = &Serial;  

  in = new AudioFileSourceSD();
  in -> RegisterMetadataCB(MDCallback, (void*)"ICY");

  mp3 = new AudioGeneratorMP3();
  mp3 -> RegisterStatusCB(StatusCallback, (void*)"mp3");

  out = new AudioOutputI2S();
  out -> SetPinout( I2S_bclkPinP, I2S_wclkPinP, I2S_doutPinP );
  out -> SetGain( 0.100 );
  */
}

void Audio::play( String aname )
{
  /*
  Serial.print( F( "Playing audio file: " ) );
  Serial.println( aname );

  in -> open( aname.c_str() );

  mp3->begin( in, out );
  */
}

bool Audio::test()
{
  return true;
}

void Audio::loop()
{
  /*
  if ( ( mp3 ) && ( mp3 -> isRunning() ) ) 
  {
    if ( ! mp3 -> loop() )
    {
      Serial.print( "Audio stop" );
      mp3 -> stop();
    }
  }
  */
}
