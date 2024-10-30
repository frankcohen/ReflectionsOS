/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef WATCHFACE_H
#define WATCHFACE_H

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

#include <PNGdec.h>
#include <Arduino_GFX_Library.h>

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Arduino_GFX *gfx;
extern Battery battery;
extern TimeService timeservice;

#define transparent_color 0xFFE0

class WatchFace 
{
  public:
    WatchFace();
    
    void begin();
    void loop();
    
    void drawImageTransparent( String filename );
    void drawImage( String filename );
    void drawImagePNG( String filename);

    void drawMainFace();

  private:
    long facetime;
    File jpegFile;
    JPEGDEC jpeg;

};

#endif // WATCHFACE_H
