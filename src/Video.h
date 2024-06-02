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

#include <Arduino_GFX_Library.h>

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts

#include "FreeSerif8pt7b.h"
#include "FreeSansBold10pt7b.h"
#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

#define COLOR_BACKGROUND RGB565(115, 58, 0)
#define COLOR_LEADING RGB565(123, 63, 0)
#define COLOR_RING RGB565(234, 68, 0)
#define COLOR_TRAILING RGB565(178, 67, 0 )
#define COLOR_TEXT RGB565( 234, 67, 0 )
#define COLOR_TEXT_BACKGROUND RGB565( 79, 42, 0)
#define COLOR_TEXT_BORDER RGB565( 207, 67, 0 )

#define leftmargin 48
#define topmargin 75
#define linespacing 20
#define maxchars 16

// Definitions for TOFeyes
#define tofdiam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define maxdist 50

#define COLOR_BACKGROUND RGB565(115, 58, 0)
#define COLOR_LEADING RGB565(123, 63, 0)
#define COLOR_RING RGB565(234, 68, 0)
#define COLOR_TRAILING RGB565(178, 67, 0 )

#define COLOR_BLUE RGB565( 12, 30, 29 )

class Video
{
  public:
    Video();
    void begin();
    void loop();
    void resetStats();
    void addReadTime( unsigned long rtime );
    void startVideo( String vname );
    void stopVideo();
    void stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 );
    int getStatus();
    boolean startAtTop();
    boolean StartAtTopOrReplicate();
    void setTofEyes( bool status );
    void printCentered( int y2, String text, uint16_t color, const GFXfont * font );

  private:
    File mjpegFile;
    long ringtimer;
    boolean firsttime;
    File vidfile;
    int videoStatus;
    unsigned long vidtimer;
    
    long fontdelay;
    
    void drawTofEyes();
    VL53L5CX_ResultsData measurementData;

    float totalFrames;
    float totalReadVideo;
    float totalDecodeVideo;
    float totalShowVideo;
    float startMs;
    float totalTime;

    bool tofEyes;
};

#endif // _video_
