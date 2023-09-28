#include <Arduino.h>

// Do not build on GCC8, GCC8 has a compiler bug

#if defined(ARDUINO_ARCH_RP2040) || (__GNUC__ == 8)
void setup() {}
void loop() {}
#else
#ifdef ESP32
    #include <WiFi.h>
    #include "SPIFFS.h"
#else
    #include <ESP8266WiFi.h>
#endif


#include <AudioOutputNull.h>
#include <AudioOutputI2S.h>
#include <AudioGeneratorMIDI.h>
#include <AudioFileSourceSPIFFS.h>

AudioFileSourceSPIFFS *sf2;
AudioFileSourceSPIFFS *mid;
AudioOutputI2S *dac;
AudioGeneratorMIDI *midi;

void setup()
{
  const char *soundfont = "/1mgm.sf2";
  const char *midifile = "/furelise.mid";

  WiFi.mode(WIFI_OFF); 

  Serial.begin(115200);
  SPIFFS.begin();
  Serial.println("Starting up...\n");

  audioLogger = &Serial;
  sf2 = new AudioFileSourceSPIFFS(soundfont);
  mid = new AudioFileSourceSPIFFS(midifile);
  
  dac = new AudioOutputI2S();
  midi = new AudioGeneratorMIDI();
  midi->SetSoundfont(sf2);
  midi->SetSampleRate(22050);
  Serial.printf("BEGIN...\n");
  midi->begin(mid, dac);
}

void loop()
{
  if (midi->isRunning()) {
    if (!midi->loop()) {
      midi->stop();
    }
  } else {
    Serial.printf("MIDI done\n");
    delay(1000);
  }
}

#endif
