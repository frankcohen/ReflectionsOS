/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on:
 JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 
 I tried ESP32_JPEG for it's use  of SIMD instructions to speed JPEG decoding:
 https://github.com/esp-arduino-libs/ESP32_JPEG
 It works. However, it is also not open-source from Espressif. And, memory
 allocation happens within the pre-compiled library. Then I had a phone call
 with Larry Bank (@bitbank2). He is an amazing person and engineer. He added
 support for SIMD to JPEGDEC. I switched back to JPEGDEC.

 SIMD is Single Instruction, Multiple Data. They are a type of processor 
 instruction that allow a single instruction to operate on multiple data 
 elements simultaneously, significantly improving performance in tasks 
 involving repetitive operations on large datasets. 
 https://www.reddit.com/r/esp32/comments/1hm91t0/comment/mm6dano/?context=3
 Only ESP32-S3 has SIMD instructions at the time of this writing.
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
    bool begin();
    bool start( File input );
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
