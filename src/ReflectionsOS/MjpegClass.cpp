/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 This is a wrapper for the Espresif provided JPEG decoder class.

 This code only runs on ESP32-S3 due to its support of SIMD.
*/

#include "MjpegClass.h"

extern Video video;

/*******************************************************************************
 * Mjpeg.cpp
 * ESP32_JPEG Wrapper Class
 ******************************************************************************/

#include "MjpegClass.h"

bool MjpegClass::begin()
{
  if ( ! firsttime ) return true;

  stime = millis();

  _inputindex = 0;

  _mjpeg_buf = (uint8_t *) malloc( MJPEG_BUFFER_SIZE );
  if ( ! _mjpeg_buf )
  {
    Serial.println( F("mjpeg_buf malloc failed, stopping" ) );
    video.stopOnError( "Video buffer", "fail", "", "", "" );
  }

  _output_buf = (uint16_t *) heap_caps_aligned_alloc( 16, MJPEG_OUTPUT_SIZE, MALLOC_CAP_8BIT );
  if (! _output_buf)
  {
    Serial.println(F("output_buf malloc failed"));
    video.stopOnError( "Video output", "buffer", "fail", "", "" );
  }
  _output_buf_size = MJPEG_OUTPUT_SIZE;

  _read_buf = (uint8_t *) malloc( READ_BUFFER_SIZE );
  if ( !_read_buf )
  {
    Serial.println( F("_read_buf malloc failed") );
    return false;
  }

  video.addReadTime( millis() - stime );

  _output_buf_size = MJPEG_OUTPUT_SIZE;
  _mjpeg_buf_offset = 0;
  _inputindex = 0;
  _remain = 0;
  
  firsttime = false;

  return true;
}

bool MjpegClass::start( File input )
{
  _input = input;
}

uint16_t * MjpegClass::getOutputbuf()
{
  return _output_buf;
}

bool MjpegClass::readMjpegBuf()
{
  if (_inputindex == 0)
  {
    _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
    _inputindex += _buf_read;
    video.addReadTime( millis() - stime );
  }

  _mjpeg_buf_offset = 0;
  int i = 0;
  bool found_FFD8 = false;

  while ((_buf_read > 0) && (!found_FFD8))
  {
    i = 0;
    while ((i < _buf_read) && (!found_FFD8))
    {
      if ((_read_buf[i] == 0xFF) && (_read_buf[i + 1] == 0xD8)) // JPEG header
      {
        found_FFD8 = true;
      }
      ++i;
    }
    if (found_FFD8)
    {
      --i;
    }
    else
    {
      _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
    }
  }

  video.addReadTime( millis() - stime );

  uint8_t *_p = _read_buf + i;
  _buf_read -= i;
  bool found_FFD9 = false;
  if (_buf_read > 0)
  {
    i = 3;
    while ((_buf_read > 0) && (!found_FFD9))
    {
      if ((_mjpeg_buf_offset > 0) && (_mjpeg_buf[_mjpeg_buf_offset - 1] == 0xFF) && (_p[0] == 0xD9)) // JPEG trailer
      {
        found_FFD9 = true;
      }
      else
      {
        while ((i < _buf_read) && (!found_FFD9))
        {
          if ((_p[i] == 0xFF) && (_p[i + 1] == 0xD9)) // JPEG trailer
          {
            found_FFD9 = true;
            ++i;
          }
          ++i;
        }
      }

      memcpy(_mjpeg_buf + _mjpeg_buf_offset, _p, i);
      _mjpeg_buf_offset += i;
      size_t o = _buf_read - i;
      if (o > 0)
      {
        memcpy(_read_buf, _p + i, o);
        _buf_read = _input.read(_read_buf + o, READ_BUFFER_SIZE - o);
        _p = _read_buf;
        _inputindex += _buf_read;
        _buf_read += o;
      }
      else
      {
        _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
        _p = _read_buf;
        _inputindex += _buf_read;
      }
      i = 0;
    }
    if (found_FFD9)
    {
      return true;
    }
  }

  video.addReadTime( millis() - stime );

  return false;
}

bool MjpegClass::decodeJpg()
{
  _remain = _mjpeg_buf_offset;

  jpeg_dec_config_t config = {
      .output_type = JPEG_RAW_TYPE_RGB565_BE,
      .rotate = JPEG_ROTATE_0D,
  };

  _jpeg_dec = jpeg_dec_open(&config);

  _jpeg_io = (jpeg_dec_io_t *)calloc(1, sizeof(jpeg_dec_io_t));

  _out_info = (jpeg_dec_header_info_t *)calloc(1, sizeof(jpeg_dec_header_info_t));

  _jpeg_io->inbuf = _mjpeg_buf;
  _jpeg_io->inbuf_len = _remain;

  jpeg_dec_parse_header(_jpeg_dec, _jpeg_io, _out_info);

  _w = _out_info->width;
  _h = _out_info->height;

  if ((_w * _h * 2) > _output_buf_size)
  {
    return false;
  }

  _jpeg_io->outbuf = (unsigned char *) _output_buf;

  jpeg_dec_process(_jpeg_dec, _jpeg_io);
  jpeg_dec_close(_jpeg_dec);

  free(_jpeg_io);
  free(_out_info);

  return true;
}

int16_t MjpegClass::getWidth()
{
  return _w;
}

int16_t MjpegClass::getHeight()
{
  return _h;
}

