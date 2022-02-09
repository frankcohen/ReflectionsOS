/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the video display.

Notes:

Using Arduino_GFX class by @moononournation
https://github.com/moononournation/Arduino_GFX
For speed and support of MPEG video

Thank you Pawel A. Hernik (https://youtu.be/o3AqITHf0mo) for tips on how to stream
raw/uncompressed video to the display.

Thank you Earle F. Philhower, III, earlephilhower@yahoo.com, for the WAV audio
player library at https://github.com/earlephilhower/ESP8266Audio

Note: To play 240x240 MJPEG uncompressed files requires the audio at
 a sample rate of 8000 kHz, 16 bits, stereo format, WAVE format

*/

#ifndef _video_
#define _video_

#include "Arduino.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "config.h"

#define MJPEG_FILENAME ".mjpeg"

class Video
{
  public:
    Video();
    void begin();
    void loop();

    unsigned long addTotal_show_video( unsigned long msec );
    boolean needsSetup();
    void clearNeedsSetup();
    void setNeedsSetup( boolean mns );
    boolean needsPlay();
    void clearNeedsPlay();
    uint8_t * getMjpegBuf();

    boolean getFirsttime();
    void setFirsttime( boolean ft );
    File getMjpegFile();
    void setMjpegFile( File thefile );

    boolean getReadyForNextMedia();
    void setReadyForNextMedia( boolean nm );

  private:
    unsigned long _total_show_video;

    long _timer;
    File _mjpegFile;

    boolean _firsttime;
    boolean _readyForNextVideo;

    boolean _needsSetupFlag;
    boolean _needsPlayFlag;

    long _total_frames;
    unsigned long _total_read_video;
    unsigned long _total_decode_video;
    unsigned long _start_ms;
    unsigned long _curr_ms;
};

#endif // _video_
