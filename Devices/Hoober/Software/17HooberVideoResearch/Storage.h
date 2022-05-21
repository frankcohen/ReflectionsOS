/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for storage tasks, including getting media/data from Cloud City server, over Wifi and Bluetooth
get TAR over Wifi, Bluetooth, mostly SD utils like print to, eventually send message to other devices

*/

#ifndef _storage_
#define _storage_

#include "Arduino.h"

#include "FS.h"
#include <SD.h>

#include "HTTPClient.h"
#include "config.h"
#include "ArduinoJson.h"

class Storage
{
  public:
    Storage();

    void begin();
    void loop();

    // Wifi services
    bool fileAvailableForDownload();
    bool findOneFile();
    bool getFileSaveToSD( String thedoc );
    String getFileListString();
    void replicateServerFiles();

    // Utils
    void smartDelay(unsigned long ms);

    // SD file utilities
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    void createDir(fs::FS &fs, const char * path);
    void removeDir(fs::FS &fs, const char * path);
    void readFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void appendFile(fs::FS &fs, const char * path, const char * message);
    void renameFile(fs::FS &fs, const char * path1, const char * path2);
    void deleteFile(fs::FS &fs, const char * path);
    void availSpace();
    void removeFiles(fs::FS &fs, const char * dirname, uint8_t levels);
    void removeDirectories(fs::FS &fs, const char * dirname, uint8_t levels);

    void extract_files( String tarfilename );
    void rm( File dir, String tempPath );
    void rmRfStar();

  private:
    String _fileName;
};

#endif // _storage_
