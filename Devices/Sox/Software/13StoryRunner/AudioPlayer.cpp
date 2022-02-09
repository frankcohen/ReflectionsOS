/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for audio tasks.

*/

#include "Arduino.h"
#include "AudioPlayer.h"
#include "Audio.h"
#include "config.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

AudioPlayer::AudioPlayer() {}

void AudioPlayer::begin()
{
  _audio.setPinout( I2S_BCLK /* bclkPin */, I2S_LRC /* wclkPin */, I2S_DOUT /* doutPin */);
  _audio.setVolume(21); // 0...21
}

void AudioPlayer::start( String filename )
{
  Serial.print( "Starting audio " );
  Serial.println( filename );

  File af = SD.open( filename, FILE_READ );
  if ( !af )
  {
      Serial.print( "Error on opening audio file " );
      Serial.print( filename );
      return;
  }

  _audio.connecttoFS(SD, af.name() );
}

void AudioPlayer::loop()
{
  _audio.loop();
}
