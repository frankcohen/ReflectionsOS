/*
Reflections, distributed entertainment experience

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

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "config.h"
#include "MjpegClass.h"
#include "Logger.h"
#include "TOF.h"
#include "Battery.h"
#include "Wifi.h"
#include "icons.h"

#include <ArduinoJson.h>

#include <Arduino_GFX_Library.h>

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts

#include "FreeSerif8pt7b.h"
#include "FreeSansBold10pt7b.h"
#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

#define leftmargin 48
#define topmargin 75
#define linespacing 20
#define maxchars 16

class Video
{
  public:
    Video();
    void begin();
    void loop();

    void stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 );
 
    void resetStats();
    void startVideo( String vname );
    void stopVideo();
    int getStatus();
    void setPaused( bool p );
    unsigned long getVideoTime();

    void addReadTime( unsigned long rtime );

  private:
    File mjpegFile;
    long ringtimer;
    bool firsttime;
    File vidfile;
    int videoStatus;
    unsigned long vidtimer;
    unsigned long videoStartTime;
    bool paused;

    File showDirectoryIterator;
    bool findMore;
    int twice;
    bool showIteratorFlag;
    File showDirectory;
    File showDir;
    File show;

    float totalFrames;
    float totalReadVideo;
    float totalDecodeVideo;
    float totalShowVideo;
    float startMs;
    float totalTime;

    unsigned long checktime;
    
    String nextVideo;
    String nextAudio;
    String nextDir;

    String showTitle;
    String showName;
    String onStartEventName;
    String showingEventNumber;
    String showingAudioFile;
    String showingVideoFile;

    int playerStatus;
};

#endif // _video_
