/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Wrapper class for JPEGDEC to stream video to the display

 Buffers: 1) GFX primary and gfxbuffer, 2) JPEGDEC

 Depends on:
 JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 
 I tried ESP32_JPEG for it's use  of SIMD instructions to speed JPEG decoding:
 https://github.com/esp-arduino-libs/ESP32_JPEG
 It works. However, it is also not open-source from Espressif. And, memory
 allocation happens within the pre-compiled library. Then I had a phone call
 with Larry Bank (@bitbank2) of the JPEGDEC project. He is an amazing person 
 and engineer. He added support for SIMD to JPEGDEC. I switched back to JPEGDEC.

 SIMD is Single Instruction, Multiple Data. They are a type of processor 
 instruction that allow a single instruction to operate on multiple data 
 elements simultaneously, significantly improving performance in tasks 
 involving repetitive operations on large datasets. 
 https://www.reddit.com/r/esp32/comments/1hm91t0/comment/mm6dano/?context=3
 Only ESP32-S3 has SIMD instructions at the time of this writing.
*/

#ifndef _MJPEGRUNNER_H_
#define _MJPEGRUNNER_H_

#include "Arduino.h"
#include "config.h"
#include <JPEGDEC.h>
#include <Arduino_GFX_Library.h>

#define READ_BUFFER_SIZE 1024
#define MAXOUTPUTSIZE (MAX_BUFFERED_PIXELS / 16 / 16)
#define MJPEG_OUTPUT_SIZE (240 * 240 * 2)      // memory for a output image frame
#define MJPEG_BUFFER_SIZE ( ( 240 * 240 * 2 ) / 10) // memory for a single JPEG frame

extern Arduino_GFX *gfx;

class MjpegRunner
{
  public:
    MjpegRunner();

    void begin();
    bool start( Stream *input );
    bool readMjpegBuf();
    bool drawJpg();

  private:
    Stream * _input;

    uint16_t * _output_buf;
    size_t _output_buf_size;
    
    uint8_t * _read_buf;
    int32_t _mjpeg_buf_offset;
    int32_t _inputindex;
    int32_t _readcount;
    int32_t _remain;

    uint8_t * _mjpeg_buf; 

    JPEGDEC _jpeg;
};

#endif // _MJPEGRUNNER_H_
