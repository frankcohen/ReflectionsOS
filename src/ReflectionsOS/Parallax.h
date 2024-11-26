/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _Parallax_
#define _Parallax_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Video.h"

#include "AccelSensor.h"

#include <Arduino_GFX_Library.h>
#include <JPEGDEC.h>

#include <SPI.h>
#include <SD.h>

// Defined in ReflectionsOfFrank.ino
extern LOGGER logger;
extern AccelSensor accel;
extern Video video;
extern Arduino_GFX *gfx;

#define COLOR_BACKGROUND RGB565(115, 58, 0)

#define accxleft 0.62
#define accxright 1.54
#define accpositions 5
#define stepvalue 0.184

class Parallax
{
  public:
    Parallax();
    void begin();
    void loop();
    uint8_t* loadFileToBuffer( String filePath );

  private:
    long ParallaxWaitTime;
    int pictureNum;
    JPEGDEC jpeg;
    size_t fileSize;

};

#endif // _Parallax_
