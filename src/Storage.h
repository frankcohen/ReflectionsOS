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
#include "HTTPClient.h"
#include "config.h"
#include "Video.h"
#include "ArduinoJson.h"
#include "secrets.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFiUdp.h>
#include <WiFiMulti.h>
#include "Logger.h"

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
    boolean listDir(fs::FS &fs, const char * dirname, uint8_t levels, bool monitor);
    boolean createDir(fs::FS &fs, const char * path);
    boolean removeDir(fs::FS &fs, const char * path);
    boolean readFile(fs::FS &fs, const char * path);
    boolean writeFile(fs::FS &fs, const char * path, const char * message);
    boolean appendFile(fs::FS &fs, const char * path, const char * message);
    boolean renameFile(fs::FS &fs, const char * path1, const char * path2);
    boolean deleteFile(fs::FS &fs, const char * path);
    boolean removeFiles(fs::FS &fs, const char * dirname, uint8_t levels);
    void availSpace();
    boolean testFileIO(fs::FS &fs, const char * path);
    boolean testNandStorage();
    void setMounted( bool mounted );
    void extract_files( String tarfilename );
    void sizeNAND();    

  private:
    String _fileName;
    boolean SDMounted = false;

    int lvlcnt;
    int DeletedCount;
    int FolderDeleteCount;
    int FailCount;
    String rootpath;
};

#endif // _storage_
