/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _LOGGER_
#define _LOGGER_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "HTTPClient.h"
#include <WiFi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum characters to buffer before writing to a log
#define log_max_size 1000

// Maximum characters in one log message
#define maxlogmsg 500

class LOGGER
{
  public:
    LOGGER();
    void begin();
    void loop();

    void info( String message );
    void warning( String message );
    void error( String message );
    void critical( String message );
    void clearLog( String logname );
    bool scanLogNumbers();
    char * urlencode(const char *str);

    void setEchoToSerial( bool echo );
    void setEchoToServer( bool echo );

  private:
    void logit( String msgtype, String msg );
    bool setActiveFile( int atvnum );
    bool sendToServer( String logfilename );
    void appendToBuffer( String data );
    void writeBufferToFile();

    char upBuffer[101];
    int bytesRead;
    String data;

    char buffer[ log_max_size ];   // Buffer to hold logged data before being saved to a log file, all at once
    int bufferIndex;               // Index to track position in the buffer

    int lowLogNumber;
    int highLogNumber;

    File mylog;
    bool mylogopen;
    String mylogname;

    std::string devname;
    String devicename;

    File myupload;
    String uploadfilename;
    bool uploading;
    long uploadchecktime;
    long uploadpacetime;
    long logcreatepacetime;
    long uploadcount;
    long deletetime;
    bool deleting;

    HTTPClient http;
    String uploadstr;

    int forcecount;

    bool echoSerial;
    bool echoServer;

};

#endif // _LOGGER_
