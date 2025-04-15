/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef WATCHFACEBASE_H
#define WATCHFACEBASE_H

#include "Arduino.h"
#include "SD.h"

#include "config.h"

#include <Arduino_GFX_Library.h>
#include <PNGdec.h>
#include <ESP32_JPEG_Library.h>
#include "MjpegClass.h"

// #include <sys/unistd.h>

extern Arduino_GFX *gfx;
extern Arduino_Canvas *bufferCanvas;
extern MjpegClass mjpeg;

void *myOpen(const char* filename, int32_t *size);
void myClose(void *handle);
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length);
int32_t mySeek(PNGFILE *handle, int32_t position);
void PNGDraw( PNGDRAW *pDraw );

class WatchFaceBase 
{
  public:
    WatchFaceBase();

    virtual void begin();
    virtual void loop();

    virtual void start(); // Method to clear the buffer
    virtual void show();

    void drawImageFromFile( String filename, bool embellishfilename, int16_t x, int16_t y);

  protected:
    String mef;
    unsigned long total_decode_video;
    unsigned long curr_ms;
    int x, y, w, h;

};

#endif // WATCHFACEBASE_H
