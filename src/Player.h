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

#ifndef _player_
#define _player_

#include <SPI.h>
#include "SD.h"
#include "ArduinoJson.h"
#include "Logger.h"
#include "Video.h"
#include "Audio.h"
#include "Storage.h"
#include "Accelerometer.h"

class Player
{
  public:
    Player();

    void begin();
    void loop();

    void openShow();
    bool findNext();
    bool tarsExist();

    void RetreatGesture();
    void SkipGesture();
    void PauseGesture();
    void DeleteGesture();

    void ShowMenu();

  private:
    unsigned long checktime;
    
    int sequence;
    bool fileValid;

    File showDirectoryIterator;
    bool findMore;
    int twice;
    bool showIteratorFlag;
    File showDirectory;
    File showDir;
    File show;

    String nextVideo;
    String nextAudio;
    String nextDir;

    String showTitle;
    String showName;
    String onStartEventName;
    String showingEventNumber;
    String showingAudioFile;
    String showingVideoFile;

    int playerStatus;
};

#endif // _player_
