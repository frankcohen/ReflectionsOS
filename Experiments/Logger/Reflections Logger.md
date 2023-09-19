ESP32+SD/NAND fails on reuse of File object to secondary file

Hi Arduinoistas, the ESP32 SD library stops writing to a file when I re-use a File object.

I wrote a logger. It runs on an ESP32-S3, stores the log values on local storage temporarily, and sends the log values over Wifi to a service for storage. My goal is to have an unlimited number of log entries. And not bog-down the ESP32 as it stores and uploads the logs.

The logger takes String values, saves them to a log file on an SD/NAND device. NAND is a surface mount version of an SD card. The logger keeps logging to the file until the file size hits 200 bytes (200 is for testing, the NAND has 8 Gbytes of storage). The logger then closes the file and marks it for upload to the server. It opens a second log file for writing. While it uploads the first log file contents to a service, it logs new values to the second file. Once the logger finishes uploading the first log file, it removes and then recreates the first file. When the second file has 200 bytes it switches logging to the first file. 

I am seeing after switching log files the write fails. I check the file size before and after the write. The File print command returns the value 40. This is the correct length of the log I am writing to the file. The file size does not change.

Up until the failure, the SD library correctly writes the log values to the log files.

Any help with this would be great of you and appreciated, including criticisms of the design and refactoring ideas. I'm open to all.

This is from my Reflections open-source project at https://github.com/frankcohen/ReflectionsOS. 

Datasheet for the SD/NAND is at https://www.lcsc.com/product-detail/NAND-FLASH_XTX-XTSD01GLGEAG_C558837.html. The SD/NAND works fine for the other parts of my project, including saving 5 megabyte or larger binary files.

Here is what I am seeing in the Serial Monitor:

```
SD card mounted
Wifi begin
Logged to /log1.txt, logval = 40, characters 40, file size before 20, file size after 40, FAIL
Logged to /log1.txt, logval = 40, characters 40, file size before 40, file size after 80, OK
Logged to /log1.txt, logval = 40, characters 40, file size before 80, file size after 120, OK
Logged to /log1.txt, logval = 40, characters 40, file size before 120, file size after 160, OK
Logged to /log1.txt, logval = 40, characters 40, file size before 160, file size after 200, OK
Switching to log 0, mylog.size() = 200
opening mylog using /log0.txt
After switch mylog.size() = 20, OK
sendToServer: CALLIOPE-00,1,Info,testing the logger 0
sendToServer: CALLIOPE-00,1,Info,testing the logger 1
Logged to /log0.txt, logval = 40, characters 40, file size before 20, file size after 60, OK
sendToServer: CALLIOPE-00,1,Info,testing the logger 2
sendToServer: CALLIOPE-00,1,Info,testing the logger 3
sendToServer: CALLIOPE-00,1,Info,testing the logger 4
Logged to /log0.txt, logval = 40, characters 40, file size before 60, file size after 100, OK
Uploading complete, logs sent to service 5
Logged to /log0.txt, logval = 40, characters 40, file size before 100, file size after 140, OK
Logged to /log0.txt, logval = 40, characters 40, file size before 140, file size after 180, OK
Logged to /log0.txt, logval = 40, characters 40, file size before 180, file size after 220, OK
Switching to log 1, mylog.size() = 220
opening mylog using /log1.txt
After switch mylog.size() = 20, OK
sendToServer: 01234567890123456789CALLIOPE-00,0,Info,testing the logger 5
sendToServer: CALLIOPE-00,0,Info,testing the logger 6
sendToServer: CALLIOPE-00,0,Info,testing the logger 7
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
sendToServer: CALLIOPE-00,0,Info,testing the logger 8
sendToServer: CALLIOPE-00,0,Info,testing the logger 9
Uploading complete, logs sent to service 5
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
Logged to /log1.txt, logval = 41, characters 41, file size before 20, file size after 20, FAIL
```

Contents of the sketch.

```
long mylogtime = millis();
int logcount = 0;

void setup()
{
  . . .
  logger.begin();
  . . .
}

void loop()
{

  if ( (millis() - mylogtime) > 2000 ) 
  {
    mylogtime = millis();
    String myl = "testing the logger ";
    myl += String(logcount++);
    logger.info( myl );    

    //storage.listDir( SD, "/", 100, true );
  }

  logger.loop();
}
```

Contents of Logger.h

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

#include <UrlEncode.h>

#define log_size_upload 200

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

  private:
    void logit( String msg );
    bool setActiveFile( int atvnum );
    bool sendToServer( String message );

    int activelog;

    File mylog;
    bool mylogopen;
    String mylogname;

    std::string devname;
    String devicename;

    File myupload;
    String uploadfilename;
    bool myuploadopen;
    bool uploading;

    long uploadcount;

};

#endif // _LOGGER_
```

Contents of Logger.cpp

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
  logmsg += activelog;
  logmsg += ",";
  logmsg += msg;
  logmsg += "\n";

  long mysize = (long) mylog.size();

  int logval = mylog.print( logmsg );
  mylog.flush();

  Serial.print( "Logged to " );
  Serial.print( mylogname );
  Serial.print( ", logval = ");
  Serial.print( logval );
  Serial.print( ", characters " );
  Serial.print( logmsg.length() );
  Serial.print( ", file size before " );
  Serial.print( mysize );
  Serial.print( ", file size after " );
  Serial.print( mylog.size() );
  if ( mylog.size() == mysize + logmsg.length() )
  {
    Serial.println( ", OK" );
  }
  else
  {
    Serial.println( ", FAIL" );
  }
  
  if ( ( uploading ) || ( mylog.size() < log_size_upload ) ) return;

  if ( activelog == 0 )
  {
    Serial.print( "Switching to log 1, mylog.size() = " );
    Serial.println( mylog.size() );

    uploadfilename = log_file_name0;
    activelog = 1;
    mylogname = log_file_name1;
  }
  else
  {
    Serial.print( "Switching to log 0, mylog.size() = " );
    Serial.println( mylog.size() );

    uploadfilename = log_file_name1;
    activelog = 0;
    mylogname = log_file_name0;
  }

  mylog.close();
  mylogopen = 0;

  Serial.print( "opening mylog using ");
  Serial.println( mylogname );

  mylog = SD.open( mylogname, FILE_WRITE );
  if( !mylog )
  {
    Serial.print( F( "Failed to open log file " ) );
    Serial.println( mylogname );
    return;
  }

  mylogopen = 1;

  // Set up for an upload to the log service

  if ( myuploadopen )
  {
    myupload.close();
    Serial.println( "Closed myupload");
  }
  
  myupload = SD.open( uploadfilename );
  if( !myupload )
  {
    Serial.print( F("Could not open upload file for reading " ) );
    Serial.println( uploadfilename );
    return;
  }

  myuploadopen = 1;
  setActiveFile( activelog );
  uploading = 1;
  uploadcount = 0;

  // Log integrity check

  mylog.print( "0123456789" );
  mylog.print( "0123456789" );
  mylog.flush();
  Serial.print( "After switch mylog.size() = " );
  Serial.print( mylog.size() );
  if ( mylog.size() == 20 )
  {
    Serial.println( ", OK" );
  }
  else
  {
    Serial.println( ", FAIL" );
  }

}

/*
* Set active log file contents
*/

bool LOGGER::setActiveFile( int atvnum )
{
  SD.remove( LOG_ACTIVE_FILE_NAME );

  File activefile = SD.open( LOG_ACTIVE_FILE_NAME, FILE_WRITE );
  if( !activefile )
  {
    Serial.print( "Error opening " );
    Serial.print( LOG_ACTIVE_FILE_NAME );
    return false;
  }

  activefile.print( String( atvnum ) );

  activefile.close();

  return true;
}

/*
* Clear the contents of a log file
*/

void LOGGER::clearLog( String logname )
{
  SD.remove( logname );

  File loggyfile = SD.open( logname, FILE_WRITE );
  if( !loggyfile )
  {
    Serial.print( F( "Failed to create log file " ) );
    Serial.println( logname );
    return;
  }

  loggyfile.print( devicename );
  loggyfile.print( ",info,Starting new log file ");
  loggyfile.print( logname );
  loggyfile.print( "\n" );
  loggyfile.close();
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

  HTTPClient http;
  http.begin( logurl, root_ca );

  int httpCode = http.GET();

  if( httpCode != HTTP_CODE_OK )
  {
    Serial.print( F( "Server response code: " ) );
    Serial.println( httpCode );
    return false;
  }

  return true;
}

/*
* Begin picks-up the state from where we left off (from the file system)
* or creates the initial state (and file system logs)
*/

void LOGGER::begin()
{
  root_ca = ssl_cert;   // Defined in secrets.h

  activelog = 0;
  mylogopen = 0;
  myuploadopen = 0;
  uploading = 0;
  uploadcount = 0;

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  devicename = devname.c_str();

  // Read the previous state from the LOG_ACTIVE_FILE_NAME file

  File activefile = SD.open( LOG_ACTIVE_FILE_NAME );
  if ( !activefile )
  {
    // Create a new active log number file, since one doesn't exist

    activefile = SD.open( LOG_ACTIVE_FILE_NAME, FILE_WRITE );
    if( !activefile )
    {
      Serial.println( F( "Unable to create log active file" ) );
    }    

    activefile.print( String( 0 ) );
    activelog = 0;      
    activefile.close();
  }
  else
  {
    // Get the active log number from the file

    char myNum[5];
    int i = 0;
    while ( activefile.available() && ( i < 5 ) )
    {
      myNum[ i ] = activefile.read();
      i++;
    }
    myNum[ i ] = 0;
    int myVer = atoi( myNum );

    Serial.print( F( "Active log number is " ) );
    Serial.println( myVer );

    activelog = myVer;
    activefile.close();
  }

  // Create the log files if they don't exist

  File logf = SD.open( log_file_name0 );
  if ( !logf )
  {
    Serial.println( F("Create new log file 0" ) );
    logf = SD.open( log_file_name0, FILE_WRITE );
    if ( !logf )
    {
      Serial.println( F("Failed to create new log file 0" ) );
    }
  }
  logf.close();

  File logf2 = SD.open( log_file_name1 );
  if ( !logf2 )
  {
    Serial.print( F("Created new log file 1" ) );
    logf2 = SD.open( log_file_name1, FILE_WRITE );
    if ( !logf2 )
    {
      Serial.println( F("Failed to create new log file 1" ) );
    }
  }
  logf2.close();

  // Open the active log file

  if ( activelog == 0 )
  {
    mylog = SD.open( log_file_name0, FILE_WRITE );
    if( !mylog )
    {
      Serial.print( F("Could not open or create " ) );
      Serial.println( log_file_name0 );
      return;
    }
    mylogopen = 1;
    mylogname = log_file_name0;
  }
  else
  {
    mylog = SD.open( log_file_name1, FILE_WRITE );
    if( !mylog )
    {
      Serial.print( F("Could not open or create " ) );
      Serial.println( log_file_name1 );
      return;
    }

    mylogopen = 1;
    mylogname = log_file_name1;
  }
}

void LOGGER::loop()
{
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
      Serial.print( "Uploading complete, logs sent to service " );
      Serial.println( uploadcount );

      myupload.close();
      myuploadopen = 0;
      clearLog( uploadfilename );
      uploading = 0;
    }
  }
}
```

Thank you! -Frank
