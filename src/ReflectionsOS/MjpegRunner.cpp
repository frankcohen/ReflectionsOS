/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
*/

#include "MjpegRunner.h"

/* Static pixel drawing callback */

static int mjpegDrawCallback( JPEGDRAW *pDraw )
{
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);  
  return 1;
}

MjpegRunner::MjpegRunner(){}

void MjpegRunner::begin()
{
  _mjpeg_buf = (uint8_t *) malloc( MJPEG_BUFFER_SIZE );
  if ( ! _mjpeg_buf )
  {
    Serial.println( F("mjpeg_buf malloc failed, stopping" ) );
    while(1);
  }

  _read_buf = (uint8_t *) malloc( READ_BUFFER_SIZE + 8 );
  if ( !_read_buf )
  {
    Serial.println( F("_read_buf malloc failed") );
    while(1);
  }
}

bool MjpegRunner::start( Stream * _mfile )
{
  _input = _mfile;
  _inputindex = 0;
  _remain = 0;
    
  return true;
}

bool MjpegRunner::readMjpegBuf()
{
    if (_inputindex == 0)
    {
      unsigned long stime = millis();
      // Reads the first chunk of the mjpeg file
      _readcount = _input->readBytes( _read_buf, READ_BUFFER_SIZE );

      _inputindex += _readcount;
    }

    _mjpeg_buf_offset = 0;
    int i = 0;
    bool found_FFD8 = false;

    // Keep reading chunks of the mjpeg file until you find one with FFD8 (the start of a jpeg image)  

    while (( _readcount > 0 ) && ( !found_FFD8 ) )
    {
      i = 0;
      while ((i < _readcount) && (!found_FFD8))
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
        _readcount = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
      }
    }

    // Now keep reading until finding the end of the jpeg FFD9

    uint8_t *_p = _read_buf + i;
    _readcount -= i;
    bool found_FFD9 = false;
    
    if ( _readcount > 0 )
    {
      i = 3;
      while ( ( _readcount > 0 ) && ( !found_FFD9) )
      {
        if ( ( _mjpeg_buf_offset > 0 ) && ( _mjpeg_buf[ _mjpeg_buf_offset - 1 ] == 0xFF) && (_p[0] == 0xD9)) // JPEG trailer
        {
          //Serial.printf("Found FFD9 at: %d.\n", i);
          found_FFD9 = true;
        }
        else
        {
          while ((i < _readcount) && (!found_FFD9))
          {
            if ((_p[i] == 0xFF) && (_p[i + 1] == 0xD9)) // JPEG trailer
            {
              found_FFD9 = true;
              ++i;
            }
            ++i;
          }
        }

        // A special check to avoid crashing on badly formed mjpeg files
        if ( _mjpeg_buf_offset > 8000 )
        {
          Serial.print( "_mjpeg_buf_offset > 8000 " );
          Serial.println( _mjpeg_buf_offset );
          return false;
        }

        if ( _mjpeg_buf == 0 )
        {
          return false;
        }

        memcpy(_mjpeg_buf + _mjpeg_buf_offset, _p, i);

        _mjpeg_buf_offset += i;
        size_t o = _readcount - i;

        if (o > 0)
        {
          memcpy(_read_buf, _p + i, o);

          _readcount = _input->readBytes(_read_buf + o, READ_BUFFER_SIZE - o);

          _p = _read_buf;
          _inputindex += _readcount;
          _readcount += o;
        }
        else
        {
          _readcount = _input->readBytes(_read_buf, READ_BUFFER_SIZE);

          if ( _readcount == 0 )
          {
            return false;
          }

          _p = _read_buf;
          _inputindex += _readcount;
        }

        i = 0;
      }
      
      if (found_FFD9)
      {
        return true;
      }
    }

    return false;
}

bool MjpegRunner::drawJpg()
{
    _remain = _mjpeg_buf_offset;
    _jpeg.openRAM( _mjpeg_buf, _remain, mjpegDrawCallback );
    _jpeg.setPixelType(RGB565_BIG_ENDIAN);
    _jpeg.decode( 0, 0, 0);     // int x, int y, int iOptions
    _jpeg.close();

    return true;
}
