 
#include "AudioTools.h"

float pitch_shift = 1.5;
int buffer_size = 1000;
uint16_t sample_rate=44100;
uint8_t channels = 1;                                      // The stream will have 2 channels 
SineWaveGenerator<int16_t> sineWave(32000);                // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave);             // Stream generated from sine wave
CsvOutput<int16_t> out(Serial, 1); 
//use one of VariableSpeedRingBufferSimple, VariableSpeedRingBuffer, VariableSpeedRingBuffer180 
PitchShiftOutput<int16_t, VariableSpeedRingBuffer<int16_t>> pitchShift(out);
StreamCopy copier(pitchShift, sound);                       // copies sound to out

// Arduino Setup
void setup(void) {  
  // Open Serial 
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // Define CSV Output
  auto config = out.defaultConfig();
  config.sample_rate = sample_rate; 
  config.channels = channels;
  out.begin(config);

  auto pcfg = pitchShift.defaultConfig();
  pcfg.copyFrom(config);
  pcfg.pitch_shift = pitch_shift;
  pcfg.buffer_size = buffer_size;
  pitchShift.begin(pcfg);

  // Setup sine wave
  sineWave.begin(channels, sample_rate, N_B4);
  Serial.println("started...");
}

// Arduino loop - copy sound to out 
void loop() {
  copier.copy();
}
