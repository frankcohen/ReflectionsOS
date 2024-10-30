/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _USBFLASHDRIVE_
#define _USBFLASHDRIVE_

#include "config.h"
#include "secrets.h"

#include "SD.h"
#include "SPI.h"

class USBFlashDrive
{
  public:
    USBFlashDrive();
    void begin();
    void loop();
    
  private:
};

#endif // _USBFLASHDRIVE_
