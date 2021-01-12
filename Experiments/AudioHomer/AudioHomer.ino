#include <Arduino.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"
#include <SD.h>

AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;

const uint8_t chipSelect = 5;
const uint8_t cardDetect = 35;
bool alreadyBegan = false;  // SD.begin() misbehaves if not first call

void setup()
{
  Serial.begin(115200);

  audioLogger = &Serial;
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S();
  out -> SetPinout( 26 /* bclkPin */, 25 /* wclkPin */, 33 /* doutPin */);

  out -> SetGain( 0.300 );

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
