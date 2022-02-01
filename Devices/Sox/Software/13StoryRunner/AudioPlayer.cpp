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

AudioPlayer::AudioPlayer()
{
  _readyForNextAudio = true;
}

void AudioPlayer::begin()
{
  _audio.setPinout( I2S_BCLK /* bclkPin */, I2S_LRC /* wclkPin */, I2S_DOUT /* doutPin */);
  _audio.setVolume(21); // 0...21
}

void AudioPlayer::loop()
{
  if ( _audioDir == NULL )
  {
    _audioDir = SD.open("/");
  }

  if ( _readyForNextAudio )
  {
    _audioFile = _audioDir.openNextFile();
    if ( _audioFile )
    {
      if (
        ( String( _audioFile.name()).endsWith(".mp3") )
        && ( ! _audioFile.isDirectory() )
        && ( ! String( _audioFile.name()).startsWith("/._") )
      )
      {
        Serial.printf_P(PSTR("Found audio '%s' from SD card\n"), _audioFile.name());
        _audio.connecttoFS(SD, _audioFile.name() );
        _readyForNextAudio = false;
      }
      return;
    }
    else
    {
      Serial.println(F("End of root directory for audio"));
      _audioDir.close();
      _readyForNextAudio = true;
      return;
    }
  }
  else
  {
    _audio.loop();
  }
}

void AudioPlayer::nextAudio( boolean ready )
{
  _readyForNextAudio = ready;
}
