#include "AudioTools.h"

uint32_t sampling_rate = 44100;
uint32_t delay_us = AudioUtils::toTimeUs(sampling_rate);
uint32_t count;
TimerAlarmRepeating timer;

void callback(void*ptr){
  count++;  
}

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  Serial.print("Delay us: ");
  Serial.println(delay_us);
}

void loop() {
  count = 0;
  timer.begin(callback, delay_us, US);
  delay(10000);
  timer.end();
  Serial.print("Sampling Rate 44100 vs eff: ");
  Serial.println(count / 10);
}