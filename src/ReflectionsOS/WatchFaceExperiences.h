/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef WATCHFACEEXPERIENCES_H
#define WATCHFACEEXPERIENCES_H

#include "config.h"
#include "secrets.h"

#include "WatchFaceMain.h"

#include <Arduino_GFX_Library.h>

extern WatchFaceMain watchfacemain;

enum WatchFaces {
    MAIN,
    MINIMAL,
    MOON,
    BLAIR
};

class WatchFaceExperiences 
{
  public:
    WatchFaceExperiences();
    void begin();
    void loop();

  private:
    int currentface;
};

#endif // WATCHFACEEXPERIENCES_H

