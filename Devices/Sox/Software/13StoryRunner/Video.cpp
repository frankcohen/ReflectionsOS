/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the video display.

*/

#include "Arduino.h"
#include "Video.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "config.h"

Video::Video()
{
  _readyForNextVideo = true;
  _timer = 5;

  _needsSetupFlag = false;
  _needsPlayFlag = false;
}

void Video::begin()
{
  _total_frames = 0;
  _total_read_video = 0;
  _total_decode_video = 0;
  _total_show_video = 0;
  _start_ms = 0;
  _curr_ms = 0;
  _firsttime = true;
}

boolean Video::needsSetup()
{
  return _needsSetupFlag;
}

void Video::clearNeedsSetup()
{
  _needsSetupFlag = false;
}

boolean Video::needsPlay()
{
  return _needsPlayFlag;
}

void Video::clearNeedsPlay()
{
  _needsPlayFlag = false;
}

boolean Video::getFirsttime()
{
  return _firsttime;
}

void Video::setFirsttime( boolean ft )
{
  _firsttime = ft;
}

boolean Video::getReadyForNextVideo()
{
  return _readyForNextVideo;
}

void Video::setReadyForNextVideo( boolean rfnv )
{
  _readyForNextVideo = rfnv;
}

File Video::getMjpegFile()
{
  return _mjpegFile;
}

unsigned long Video::addTotal_show_video( unsigned long msec )
{
  _total_show_video += msec;
  return _total_show_video;
}

void Video::loop()
{
  if ( _videoDir == NULL )
  {
    _videoDir = SD.open("/");
    if( ! _videoDir )
    {
        Serial.println("Failed to open directory");
        _videoDir == NULL;
        return;
    }
  }

  if ( _readyForNextVideo )
  {
    _mjpegFile = _videoDir.openNextFile();

    if ( _mjpegFile )
    {
      if ( ( String( _mjpegFile.name()).endsWith( MJPEG_FILENAME ) )
        && ( ! _mjpegFile.isDirectory() )
        && ( ! String( _mjpegFile.name()).startsWith(".") )
      )
      {
        Serial.printf_P(PSTR("MJPEG start '%s' from SD card...\n"), _mjpegFile.name());
        _start_ms = millis();
        _curr_ms = millis();
        _needsSetupFlag = true;
        _firsttime = false;
        _readyForNextVideo = false;
        return;
      }
      return;
    }
    else
    {
      Serial.println(F("End of root directory"));
      _videoDir.close();
      _readyForNextVideo = true;
      return;
    }
  }
  else
  {
    if ( _mjpegFile.available() )
    {
      // Read video

      if ( _timer-- == 0 )
      {
        _timer = 4;
        _needsPlayFlag = true;
        _curr_ms = millis();
        _total_frames++;
      }
    }
    else
    {
      _mjpegFile.close();
      _readyForNextVideo = true;

      int _time_used = millis() - _start_ms;
      Serial.println(F("MJPEG end"));
      float fps = 1000.0 * _total_frames / _time_used;
      _total_decode_video -= _total_show_video;
      Serial.printf("Total frames: %d\n", _total_frames);
      Serial.printf("Time used: %d ms\n", _time_used);
      Serial.printf("Average FPS: %0.1f\n", fps);
      Serial.printf("Read MJPEG: %lu ms (%0.1f %%)\n", _total_read_video, 100.0 * _total_read_video / _time_used);
      Serial.printf("Decode video: %lu ms (%0.1f %%)\n", _total_decode_video, 100.0 * _total_decode_video / _time_used);
      Serial.printf("Show video: %lu ms (%0.1f %%)\n", _total_show_video, 100.0 * _total_show_video / _time_used);
    }
  }
}
