  #include <Arduino.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"
#include <SD.h>

AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;

#define AudioPower 7

void setup()
{
  Serial.begin(115200);

  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, HIGH);

  audioLogger = &Serial;
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S();
  out -> SetPinout( 9 /* bclkPin */, 10 /* wclkPin */, 8 /* doutPin */);

  out -> SetGain( 0.600 );

  aac->begin(in, out);
}


void loop()
{
  if (aac->isRunning()) {
    aac->loop();
  } else {
    aac -> stop();
    Serial.printf("AAC done\n");
    delay(1000);
  }
}
