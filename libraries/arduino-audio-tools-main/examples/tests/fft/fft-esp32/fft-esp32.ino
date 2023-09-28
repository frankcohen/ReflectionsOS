#include "AudioTools.h"
#include "AudioLibs/AudioESP32FFT.h" // Using ESP32FFT

AudioESP32FFT fftc; // or AudioKissFFT
SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> in(sineWave);
StreamCopy copier(fftc, in);
uint16_t sample_rate = 44100;
int bits_per_sample = 16;
int channels = 1;
float value = 0;

// display fftc result
void fftcResult(AudioFFTBase &fftc) {
  float diff;
  auto result = fftc.result();
  if (result.magnitude > 100) {
    Serial.print(result.frequency);
    Serial.print(" ");
    Serial.print(result.magnitude);
    Serial.print(" => ");
    Serial.print(result.frequencyAsNote(diff));
    Serial.print(" diff: ");
    Serial.print(diff);
    Serial.print(" - time ms ");
    Serial.print(fftc.resultTime() - fftc.resultTimeBegin());
    Serial.println();

  }
}

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  // set the frequency
  sineWave.setFrequency(N_B4);

  // Setup sine wave
  auto cfg = in.defaultConfig();
  cfg.channels = channels;
  cfg.sample_rate = sample_rate;
  in.begin(cfg);

  // Setup FFT
  auto tcfg = fftc.defaultConfig();
  tcfg.copyFrom(cfg);
  tcfg.length = 4096;
  tcfg.callback = &fftcResult;
  fftc.begin(tcfg);
}

void loop() { copier.copy(); }