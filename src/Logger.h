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

#define log_size_upload 1000

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
    bool sendToServer( String message );

    char buffer[101];
    int bytesRead;
    String data;

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
