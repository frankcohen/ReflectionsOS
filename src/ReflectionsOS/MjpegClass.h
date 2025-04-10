/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 ESP32_JPEG Wrapper Class

 Depends on:
 https://github.com/esp-arduino-libs/ESP32_JPEG

 This code only runs on ESP32-S3 due to its support of SIMD.

 Previously used JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 changed to ESP32-JPEG for it's support of Single Instruction, Multiple Data, (SIMD) 
 instructions are a type of processor instruction that allows a single instruction to 
 operate on multiple data elements simultaneously, significantly improving performance 
 in tasks involving repetitive operations on large datasets. 
 https://www.reddit.com/r/esp32/comments/1hm91t0/comment/mm6dano/?context=3

*/

#ifndef _MJPEGCLASS_H_
#define _MJPEGCLASS_H_

#include "Arduino.h"
#include "config.h"
#include "Video.h"
#include <FS.h>
#include <ESP32_JPEG_Library.h>

#define MJPEG_OUTPUT_SIZE (240 * 240 * 2)          // memory for a output image frame
#define MJPEG_BUFFER_SIZE (MJPEG_OUTPUT_SIZE / 20) // memory for a single JPEG frame

#define READ_BUFFER_SIZE 1024
#define MAXOUTPUTSIZE (MAX_BUFFERED_PIXELS / 16 / 16)

class MjpegClass
{
  public:
    bool begin();
    bool start( File input );
    bool readMjpegBuf();
    bool decodeJpg();
    int16_t getWidth();
    int16_t getHeight();
    uint16_t * getOutputbuf();

  private:
    File _input;
    uint8_t * _mjpeg_buf;
    uint8_t *_read_buf;
    uint16_t *_output_buf;

    size_t _output_buf_size;
    int32_t _mjpeg_buf_offset;

    jpeg_dec_handle_t *_jpeg_dec;
    jpeg_dec_io_t *_jpeg_io;
    jpeg_dec_header_info_t *_out_info;

    int16_t _w = 0, _h = 0;

    int32_t _inputindex;
    int32_t _buf_read;
    int32_t _remain;

    bool firsttime = true;
    unsigned long stime;
};

#endif // _MJPEGCLASS_H_
