# Scalable logger ESP32 + Wifi + SD/NAND for multiple devices

I wrote a logger. It runs on an ESP32-S3, stores the log values on local storage temporarily, and sends the log values over Wifi to a service for storage and anlysis. My goal is to have an unlimited number of log entries. I do not want to bog-down the ESP32 as it stores and uploads the logs. I plan to have hundreds or thousands of devices logging concurrently. And it works even when Wifi service is not available, following a store-and-forward pattern.

![Log data on an AWS hosting service](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/Logger.jpg)

The logger receives String values, saves them to a log file on an SD/NAND device. [NAND](https://www.lcsc.com/product-detail/NAND-FLASH_XTX-XTSD01GLGEAG_C558837.html) is a surface mount version of an SD card. The logger keeps logging to the file until the file size hits 200 bytes (200 is for testing, the NAND has 8 Gbytes of storage). The logger then closes the file and starts logging to a new log file. A separately running service finds the previous log files and sends the log contents to a Web-based service. The service stores the log data in a MongoDB collection. The logger deletes log files after it completes sending them to the service.

![Log data on an AWS hosting service](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/Logger_Architecture.jpg)

This article covers the client-side logger code. It makes HTTP GET requests as described below. The server side components log to local log files, and optionally to a MongoDB collection.

## Log levels

Logger implements 4 levels of logs: info, warning, error, and critical. Call these methods to put a message into the log.

```
void info( String message );
void warning( String message );
void error( String message );
void critical( String message );
```

For example, logger.info( "My log message" ); and the log appears as
```
device-name, info, My log message
```

## Log files

Logger creates log files with the name 'log' at the root level of the SD/NAND. For example, log1, log2, log3, and more. Logger creates the next log file after it stores 200 or more bytes of log data. Adjust the size for your project, any size will do.

## Web-based API service

Logger's upload service runs every 20 seconds. It finds the log file with the lowest log number. It uses HTTPclient to make a GET request to the Web server in this format:

```
https://server.com?message=the%2Alog%2Amessage
```

Logger URL encodes the message field before sending it to the server. Logger adds a unique device identifier to the message. It uses the name of the device from a #define statement appended with the final 2 digits of the ESP32 MAC address. For example, CALLIOPE-8F, info, the log message.

By default Logger uses HTTPS protocols. The root server certificate must be made available for HTTPclient to make a connection. Logger expects to find the certificate in a secrets.h file. Set the file using: 

```
#include "secrets.h"
extern const char* root_ca;   // Defined in secrets.h
```

## Using logger in Arduino sketches and C++ files

Here is an examaple Arduino sketch using logger.

```
#include "Logger.h"

setup() {
  logger.begin();
}

loop() {
  logger.info( "My log message" );
}
```

## Bugs

While working on the logger I found two bugs in the ESP32. When the logger initially opens a new log file the SD library size() method returns garbage values. Once I write some data to the log f ile size() returns the correct value. This is [Bug Report 8625](https://github.com/espressif/arduino-esp32/issues/8625#issuecomment-1715236171) for Espressif for the ESP32.

The second issues happens when using the HTTPclient library. If the code does not use the end() method after making an HTTP GET request to the Web service, then HTTPclient causes the SD library to stop writing data to the SD/NAND device. The File object write() method returns a value equal to the number of bytes you asked it to write, yet the file size remains the same as though the write never happened. Using the HTTPclient end() method solves this problem.

## How you may help

Any help with this would be great of you and appreciated, including criticisms of the design and refactoring ideas. I'm open to all.

This is from my Reflections open-source project at https://github.com/frankcohen/ReflectionsOS. 

-Frank Cohen (fcohen@starlingwatch.com)

Source code:

***Logger.h***

```
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

#include <UrlEncode.h>

#define log_size_upload 500

#define maxlogmsg 300

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

  private:
    void logit( String msg );
    bool setActiveFile( int atvnum );
    bool sendToServer( String message );

    int lowLogNumber;
    int highLogNumber;

    File mylog;
    File lognext;
    bool mylogopen;
    String mylogname;
    long mylogtime;

    std::string devname;
    String devicename;

    File myupload;
    String uploadfilename;
    bool myuploadopen;
    bool uploading;
    long uploadchecktime;
    long uploadcount;

    HTTPClient http;

    bool forceit;
    int forcecount;

};

#endif // _LOGGER_
```

*** Logger.cpp***
```
/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on:
 https://github.com/plageoj/urlencode

Example log entry:
CALLIOPE-7A,Logger started correctly
then the node.js function adds the date
20230808-11:45AM,CALLIOPE-7A,Logger started correctly

Implemented the log receiver service in node.js using:
// GET to make a log entry
var timestamp = require('log-timestamp');
router.get( "/logit", (req, res) => {
  console.log( req.query.message );
  res.send( "<html><body>Logged</body></html>" );
}); 

requires timestamp module, install using:
npm install log-timestamp

Example,
https://cloudcity.starlingwatch.com/api/logit?message=thisismyfirstlogentry5

*/

#include "LOGGER.h"

extern const char* root_ca;   // Defined in secrets.h

LOGGER::LOGGER(){}

void LOGGER::info( String message )
{
  String lm = "Info,";
  lm += message;
  logit( lm );
}

void LOGGER::warning( String message )
{
  String lm = "Warning,";
  lm += message;
  logit( lm );
}

void LOGGER::error( String message )
{
  String lm = "Error,";
  lm += message;
  logit( lm );
}

void LOGGER::critical( String message)
{
  String lm = "Critical,";
  lm += message;
  logit( lm );
}

/*
* Adds host name and sends to active file
*/

void LOGGER::logit( String msg )
{
  if ( mylogopen != 1 )
  {
    Serial.println( "Not logging, mylog is not open" );
    return;    
  }
 
  String logmsg = devname.c_str();
  logmsg += ",";
  logmsg += msg;
  logmsg += "\n";

  long mysize = (long) mylog.size();

  int logval = mylog.print( logmsg );
  mylog.flush();

  Serial.print( "Logged to " );
  Serial.print( mylogname );
  Serial.print( ", written = ");
  Serial.print( logval );
  Serial.print( ", characters " );
  Serial.print( logmsg.length() );
  Serial.print( ", file size before " );
  Serial.print( mysize );
  Serial.print( ", file size after " );
  Serial.print( mylog.size() );
  if ( mylog.size() == mysize + logmsg.length() )
  {
    Serial.print( F( ", forcecount = " ) );
    Serial.print( forcecount );
    Serial.println( F( " OK" ));
        
    forceit = 1;
  }
  else
  {
    Serial.println( ", FAIL" );

    Serial.println( F("Forcing to new log file, forecount = " ) );
    Serial.println( forcecount++ );
    forceit = 0;
  }

  if ( forceit )
  {
    forceit = 1;

    if ( mylog.size() < log_size_upload ) return;

    if ( ! scanLogNumbers() ) return;
  }

  mylog.close();
  mylogopen = 0;

  String elname = logname_start;
  elname += String( highLogNumber + 1 );
  
  lognext = SD.open( elname, FILE_WRITE );
  if( !lognext )
  {
    Serial.print( F( "Unable to create log file " ) );
    Serial.println( elname );
  }    
  
  mylog = lognext;
  mylogname = elname;
  mylogopen = 1;
  
  Serial.print( F( "Opened log file " ) );
  Serial.print( mylogname );
  Serial.print( ", mylog.size() = " );
  Serial.println( (long) mylog.size() );

  // I need to write something to the log file here
  // because the Espressif SD library returns a random
  // value for mylog.size() before something is
  // written to the log file.

  mylog.print( devname.c_str() );
  mylog.print( ",Info," );
  mylog.print( "Starting log " );
  mylog.print( mylogname );
  mylog.print( "\n" );
  mylog.flush();
}

/*
* Sends message to the server over HTTPS GET protocol
*/

bool LOGGER::sendToServer( String message )
{
  Serial.print( "sendToServer: ");
  Serial.println( message );

  String logurl = cloudCityLogURL;
  logurl += urlEncode( message );

  http.begin( logurl, root_ca );
  http.setReuse(true);

  int httpCode = http.GET();

  if( httpCode != HTTP_CODE_OK )
  {
    Serial.print( F( "Server response code: " ) );
    Serial.println( httpCode );
    return false;
  }

  http.end();

  return true;
}

bool LOGGER::scanLogNumbers()
{
  lowLogNumber = 1000000;
  highLogNumber = 0;

  File root = SD.open( "/" );
  if( !root ){
      Serial.println( F( "scanLogNumbers failed to open directory /" ) );
      return false;
  }

  while ( 1 )
  {
    File file = root.openNextFile();
    if ( file )
    {
      if ( ! file.isDirectory() )
      {
        String myfilename = file.name();

        if ( myfilename.startsWith( "log" ) )
        {
          String mynum = myfilename.substring( 3 );
          int mynumbr = mynum.toInt();

          if ( mynumbr < lowLogNumber )
          {
            lowLogNumber = mynumbr;
          }

          if ( mynumbr > highLogNumber )
          {
            highLogNumber = mynumbr;
          }
        }
      }
    }
    else
    {
      break;
    }
  }

  return true;
}

/*
* Begin picks-up the state from where we left off (from the file system)
* or creates the initial state (and file system logs)
*/

void LOGGER::begin()
{
  Serial.println( "Logger begin");

  root_ca = ssl_cert;   // Defined in secrets.h

  mylogopen = 0;
  myuploadopen = 0;
  uploading = 0;
  uploadcount = 0;
  forceit = 0;
  forcecount = 0;

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  devicename = devname.c_str();

  // Open a new log file

  if ( ! scanLogNumbers() ) return;

  mylogname = logname_start;
  mylogname += String( highLogNumber + 1 );
  
  mylog = SD.open( mylogname, FILE_WRITE );
  if( !mylog )
  {
    Serial.print( F( "Unable to create log file " ) );
    Serial.println( mylog );
  }    
  
  mylogopen = 1;
  
  Serial.print( F( "Opened log file " ) );
  Serial.println( mylogname );

  uploadchecktime = millis();

  Serial.println( "Logger begin done");
}

void LOGGER::loop()
{

  if ( ( ( millis() - uploadchecktime ) > 10000 ) )
  {
    uploadchecktime = millis();
    
    Serial.println( "Checking for upload" );

    if ( !uploading )
    {

      if ( scanLogNumbers() )
      {
        uploadfilename = logname_start;
        uploadfilename += String( lowLogNumber );

        if ( ( mylogopen ) && ( uploadfilename.equals( mylogname ) ) )
        {
          Serial.print( F( "Skipping log file because it is open " ) );
          Serial.println( uploadfilename );
        }
        else
        {
          myupload = SD.open( uploadfilename );
          if( !myupload )
          {
            Serial.print( F( "Unable to open upload file " ) );
            Serial.println( uploadfilename );
            myuploadopen = 1;
            uploadcount = 0;
            uploading = 0;
          }
          else
          {
            uploading = 1;
            myuploadopen = 1;
            uploadcount = 0;
            
            Serial.print( F( "Opened upload file " ) );
            Serial.println( uploadfilename );
          }
        }
      }
    }
    else
    {
      Serial.println( F( "We're uploading, so I'm fine." ) );
    }
  }

  if ( uploading )
  {
    long avail = myupload.available();

    if ( avail > 0 )
    {
      char line[250];
      char fchr;

      int i=0;
      while ( i < 249 )
      {
        fchr = myupload.read();
        if ( fchr == '\n' ) break;
        line[i] = fchr;
        i++;
      }
      line[ i ] = 0;

      String mys = line;

      sendToServer( mys );
      uploadcount++;
    }
    else
    {
      Serial.print( "Uploading complete, log sent to service " );
      Serial.println( uploadcount );

      myupload.close();
      myuploadopen = 0;
      SD.remove( uploadfilename );
      uploading = 0;
    }
  }
}
```

## Find the latest update

The latest update is in the source code repository at [https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/Scalable%20logger.md](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/Scalable%20logger.md)