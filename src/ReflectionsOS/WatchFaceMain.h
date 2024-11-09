/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef WATCHFACEMAIN_H
#define WATCHFACEMAIN_H

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Video.h"
#include "TimeService.h"

#include <PNGdec.h>
#include <Arduino_GFX_Library.h>

#include "WatchFaceBase.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Battery battery;
extern TimeService timeservice;

extern Arduino_GFX *gfx;
extern Arduino_Canvas *bufferCanvas;

#define transparent_color 0xFFE0

class WatchFaceMain : public WatchFaceBase 
{
  public:
    WatchFaceMain();
    
    void begin() override;
    void loop() override;    
    
    void drawMainFace();

  private:

    // Hours and minutes animate clockwise and counterclockwist into position
    uint32_t lastUpdate;
    uint32_t animationStartTime;
    int currentHour;
    int currentMinute;
    int targetHour;
    int targetMinute;
    bool animating;

    long facetime;
    File jpegFile;
    JPEGDEC jpeg;

};

#endif // WATCHFACE_H
