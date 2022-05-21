/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for story functions, including decoding TAR data to storage,
set-up story elements like gestures, location triggers

*/

#ifndef _show_
#define _show_

#include <SPI.h>
#include "SD.h"
#include "ArduinoJson.h"

class Shows
{
  public:
    Shows();

    void begin();
    void loop();

    void openShow();

    void setReadyForNextShow( boolean rfns );
    boolean getReadyForNextShow();

    boolean getReadyForNextMedia();
    void setReadyForNextMedia( boolean nm );

    boolean findNext();

    String getNextVideo();
    String getNextAudio();

  private:
    boolean readyForNextShow;
    boolean readyForNextMedia;
    int sequence;

    File showDirectoryIterator;
    File showDirectory;
    File showDir;
    File show;

    long showTimer;
    long dirTimer;

    String nextVideo;
    String nextAudio;
    String nextDir;

    String showTitle;
    String showName;
    String onStartEventName;
    String showingEventNumber;
    String showingAudioFile;
    String showingVideoFile;
};

#endif // _show_
