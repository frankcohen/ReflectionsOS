/**
 * @file communication-codec-test.ino
 * @author Phil Schatzmann
 * @brief generate sine wave -> encoder -> decoder -> audiokit (i2s)
 * @version 0.1
 * @date 2022-04-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "AudioTools.h"
#include "AudioCodecs/CodecAPTX.h"
#include "AudioLibs/AudioKit.h"

AudioInfo info(24000, 2, 16);
SineWaveGenerator<int16_t> sineWave( 32000);  // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound( sineWave); // Stream generated from sine wave
//AudioKitStream out; 
//I2SStream out; 
AudioKitStream out; 
EncodedAudioStream decoder(&out, new APTXDecoder()); // encode and write
EncodedAudioStream encoder(&decoder, new APTXEncoder()); // encode and write
StreamCopy copier(encoder, sound);     

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // start I2S
  Serial.println("starting I2S...");
  auto cfgi = out.defaultConfig(TX_MODE);
  cfgi.copyFrom(info);
  out.begin(cfgi);

  // Setup sine wave
  auto cfgs = sineWave.defaultConfig();
  cfgs.copyFrom(info);
  sineWave.begin(info, N_B4);

  // start decoder
  decoder.begin(info);

  // start encoder
  encoder.begin(info);

  Serial.println("Test started...");
}


void loop() { 
  copier.copy();
}