#ifndef _Parallax_
#define _Parallax_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

#include "Accelerometer.h"

#include <Arduino_GFX_Library.h>
#include <JPEGDEC.h>

#include <SPI.h>
#include <SD.h>

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
