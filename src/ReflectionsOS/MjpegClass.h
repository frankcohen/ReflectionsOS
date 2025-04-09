/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on:
 JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 
*/

#ifndef _MJPEGCLASS_H_
#define _MJPEGCLASS_H_

#include "Arduino.h"
#include "config.h"
#include <JPEGDEC.h>
#include "Video.h"

#define READ_BUFFER_SIZE 1024
#define MAXOUTPUTSIZE (MAX_BUFFERED_PIXELS / 16 / 16)

class MjpegClass
{
  public:
    MjpegClass();
    bool setup(
          Stream *input, uint8_t *mjpeg_buf, JPEG_DRAW_CALLBACK *pfnDraw, bool useBigEndian,
          int x, int y, int widthLimit, int heightLimit, boolean firsttime );
    bool readMjpegBuf();
    bool drawJpg();

  private:
    Stream *_input;
    uint8_t *_mjpeg_buf;
    JPEG_DRAW_CALLBACK *_pfnDraw;
    bool _useBigEndian;
    int _x;
    int _y;
    int _widthLimit;
    int _heightLimit;

    uint8_t *_read_buf;
    int32_t _mjpeg_buf_offset;

    JPEGDEC _jpeg;
    int _scale;

    int32_t _inputindex;
    int32_t readcount;
    int32_t _remain;

    int32_t bigcounter;
};

#endif // _MJPEGCLASS_H_
