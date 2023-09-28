/**
 * @file basic-a2dp-fft-led.ino
 * @brief A2DP Sink with output of the FFT result to the LED matrix   
 * For details see the FFT Wiki: https://github.com/pschatzmann/arduino-audio-tools/wiki/FFT
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "AudioLibs/AudioRealFFT.h" // or any other supported inplementation
#include "AudioLibs/LEDOutput.h"
#include "BluetoothA2DPSink.h"

#define PIN_LEDS 22
#define LED_X 32
#define LED_Y 8

BluetoothA2DPSink a2dp_sink;
AudioRealFFT fft; // or any other supported inplementation
LEDOutput led(fft); // output to LED matrix

// Provide data to FFT
void writeDataStream(const uint8_t *data, uint32_t length) {
  fft.write(data, length);
}

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // Setup FFT
  auto tcfg = fft.defaultConfig();
  tcfg.length = 1024;
  tcfg.channels = 2;
  tcfg.sample_rate = a2dp_sink.sample_rate();;
  tcfg.bits_per_sample = 16;
  fft.begin(tcfg);

  // Setup LED matrix output
  auto lcfg = led.defaultConfig();
  lcfg.x = LED_X;
  lcfg.y = LED_Y;
  lcfg.fft_group_bin = 3;
  lcfg.fft_start_bin = 0;
  lcfg.fft_max_magnitude = 40000;
  led.begin(lcfg);

  // add LEDs
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(led.ledData(), led.ledCount());

  // register A2DP callback
  a2dp_sink.set_stream_reader(writeDataStream, false);

  // Start Bluetooth Audio Receiver
  Serial.print("starting a2dp-fft...");
  a2dp_sink.set_auto_reconnect(false);
  a2dp_sink.start("a2dp-fft");

}

void loop() { 
  led.update();
  delay(50);
}