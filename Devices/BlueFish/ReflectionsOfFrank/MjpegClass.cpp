/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
*/

#include "MjpegClass.h"

extern Video video;

#define READ_BUFFER_SIZE 1024
#define MAXOUTPUTSIZE (MAX_BUFFERED_PIXELS / 16 / 16)

MjpegClass::MjpegClass() {}

bool MjpegClass::setup(
      Stream *input, uint8_t *mjpeg_buf, JPEG_DRAW_CALLBACK *pfnDraw, bool useBigEndian,
      int x, int y, int widthLimit, int heightLimit, boolean firsttime )
{
    _input = input;
    _mjpeg_buf = mjpeg_buf;
    _pfnDraw = pfnDraw;
    _useBigEndian = useBigEndian;
    _x = x;
    _y = y;
    _widthLimit = widthLimit;
    _heightLimit = heightLimit;
    _inputindex = 0;
    _mjpeg_buf_offset = 0;
    _scale = -1;
    _remain = 0;

    bigcounter = 0;

    if ( firsttime )
    {
      unsigned long stime = millis();

      _read_buf = (uint8_t *) malloc( READ_BUFFER_SIZE + 8 );
      if ( !_read_buf )
      {
        Serial.println( F("_read_buf malloc failed") );
        while(1);
      }

      video.addReadTime( millis() - stime );
    }

    return true;
}

bool MjpegClass::readMjpegBuf()
{
    if (_inputindex == 0)
    {
      unsigned long stime = millis();
      // Reads the first chunk of the mjpeg file
      readcount = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
      _inputindex += readcount;
      video.addReadTime( millis() - stime );
    }

    _mjpeg_buf_offset = 0;
    int i = 0;
    bool found_FFD8 = false;

    // Keep reading chunks of the mjpeg file until you find one with FFD8 (the start of a jpeg image)  

    while ((readcount > 0) && (!found_FFD8))
    {
      i = 0;
      while ((i < readcount) && (!found_FFD8))
      {
        if ((_read_buf[i] == 0xFF) && (_read_buf[i + 1] == 0xD8)) // JPEG header
        {
          //Serial.print( F( "Found FFD8 at: " ) );
          //Serial.print( i );
          found_FFD8 = true;
          bigcounter++;
          //Serial.print( F( " bigcounter = " ) );
          //Serial.println( bigcounter );
        }
        ++i;
      }
      if (found_FFD8)
      {
        --i;
      }
      else
      {
        unsigned long stime = millis();
        readcount = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
        video.addReadTime( millis() - stime );
      }
    }

    // Now keep reading until finding the end of the jpeg FFD9

    uint8_t *_p = _read_buf + i;
    readcount -= i;
    bool found_FFD9 = false;
    
    if (readcount > 0)
    {
      i = 3;
      while ((readcount > 0) && (!found_FFD9))
      {
        if ((_mjpeg_buf_offset > 0) && (_mjpeg_buf[_mjpeg_buf_offset - 1] == 0xFF) && (_p[0] == 0xD9)) // JPEG trailer
        {
          //Serial.printf("Found FFD9 at: %d.\n", i);
          found_FFD9 = true;
        }
        else
        {
          while ((i < readcount) && (!found_FFD9))
          {
            if ((_p[i] == 0xFF) && (_p[i + 1] == 0xD9)) // JPEG trailer
            {
              found_FFD9 = true;
              ++i;
            }
            ++i;
          }
        }

        /*
        Serial.print( "i: " );
        Serial.println( i );
        Serial.print( "_mjpeg_buf: " );
        Serial.println( (long) _mjpeg_buf );
        Serial.print( "_mjpeg_buf_offset: " );
        Serial.println( (long) _mjpeg_buf_offset );
        Serial.print( "_p: " );
        Serial.println( (long) _p );
        */

        if ( _mjpeg_buf == 0 )
        {
          return false;
        }

        memcpy(_mjpeg_buf + _mjpeg_buf_offset, _p, i);

        _mjpeg_buf_offset += i;
        size_t o = readcount - i;

        if (o > 0)
        {
          //Serial.printf("o: %d\n", o); //frankolo
          
          memcpy(_read_buf, _p + i, o);

          unsigned long stime = millis();
          readcount = _input->readBytes(_read_buf + o, READ_BUFFER_SIZE - o);
          _p = _read_buf;
          _inputindex += readcount;
          readcount += o;
          video.addReadTime( millis() - stime );
          
          //Serial.printf("readcount: %d\n", readcount);
        }
        else
        {
          /*
          Serial.print( "Diagnose: " );
          Serial.print( o );
          Serial.print( " " );
          Serial.print( readcount );
          Serial.print( " " );
          Serial.print( i );
          Serial.print( " " );
          Serial.print( (long) _read_buf );
          Serial.print( " " );
          Serial.print( (long) _input );
          Serial.print( " " );
          Serial.print( READ_BUFFER_SIZE );
          Serial.print( " " );
          Serial.print( found_FFD9 );
          Serial.print( " " );
          */

          unsigned long stime = millis();

          readcount = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
          
          _p = _read_buf;
          _inputindex += readcount;
          video.addReadTime( millis() - stime );

          /*
          Serial.print( (long) readcount );
          Serial.print( " " );
          Serial.print( (long) _inputindex );
          Serial.println( " " );
          */
        }

        i = 0;
      }
      
      if (found_FFD9)
      {
        //Serial.println("found_FFD9");
        return true;
      }
    }

    //Serial.println("Returning false");
    return false;
}

bool MjpegClass::drawJpg()
{
    _remain = _mjpeg_buf_offset;
    _jpeg.openRAM(_mjpeg_buf, _remain, _pfnDraw);
    if (_scale == -1)
    {
      // scale to fit height
      int iMaxMCUs;
      int w = _jpeg.getWidth();
      int h = _jpeg.getHeight();
      float ratio = (float)h / _heightLimit;
      if (ratio <= 1)
      {
        _scale = 0;
        iMaxMCUs = _widthLimit / 16;
      }
      else if (ratio <= 2)
      {
        _scale = JPEG_SCALE_HALF;
        iMaxMCUs = _widthLimit / 8;
        w /= 2;
        h /= 2;
      }
      else if (ratio <= 4)
      {
        _scale = JPEG_SCALE_QUARTER;
        iMaxMCUs = _widthLimit / 4;
        w /= 4;
        h /= 4;
      }
      else
      {
        _scale = JPEG_SCALE_EIGHTH;
        iMaxMCUs = _widthLimit / 2;
        w /= 8;
        h /= 8;
      }
      _jpeg.setMaxOutputSize(iMaxMCUs);
      _x = (w > _widthLimit) ? 0 : ((_widthLimit - w) / 2);
      _y = (_heightLimit - h) / 2;
    }
    if (_useBigEndian)
    {
      _jpeg.setPixelType(RGB565_BIG_ENDIAN);
    }
    _jpeg.decode(_x, _y, _scale);
    _jpeg.close();

    return true;
}
