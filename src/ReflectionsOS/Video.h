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
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include "Minya16pt7b.h"

#include "MjpegRunner.h"        // Ensure this is included first
extern MjpegRunner mjpegrunner; // Declare the external instance

#include "Logger.h"
#include "Battery.h"
#include "Wifi.h"

extern LOGGER logger;
extern Battery battery;
extern Wifi wifi;

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts

#include "ScienceFair14pt7b.h"

#define leftmargin 48
#define topmargin 75
#define linespacing 24
#define maxchars 16

class Video
{
  public:
    Video();
    void begin();
    void beginBuffer();

    void loop();

    void stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 );
 
    void resetStats();
    void startVideo( String vname );
    void stopVideo();
    bool getStatus();
    void setPaused( bool p );
    unsigned long getVideoTime();

    void paintText( String mef );
    
    void addReadTime( unsigned long rtime );

  private:
    void addCRLF(String &s, size_t lineLen);

    File mjpegFile;
    long ringtimer;
    bool videoStatus;
    unsigned long vidtimer;
    unsigned long videoStartTime;
    bool paused;

    float totalFrames;
    float totalReadVideo;
    float totalDecodeVideo;
    float totalShowVideo;
    float startMs;
    float totalTime;
    float total_read_video;
    float total_show_video;

    int playerStatus;
    unsigned long curr_ms;

    int x, y, w, h;

};

#endif // _video_
