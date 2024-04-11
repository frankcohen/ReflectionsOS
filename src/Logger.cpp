/*
 Reflections, mobile connected entertainment experience

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

Example log entry:
CALLIOPE-7A,info,Logger started correctly
then the node.js function adds the date
20230808-11:45AM,CALLIOPE-7A,info,Logger started correctly

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
https://myserver.com/api/logit?message=thisismyfirstlogentry5

Article and comments at:
https://www.reddit.com/r/esp32/comments/16mj3zh/scalable_logger_esp32_wifi_sdnand_for_multiple/

Issure reports:
https://github.com/espressif/arduino-esp32/issues/9465
The logger fails when it uses SD.remove(“/REFLECTIONS/log1”). It works fine with log1.txt. I opened a bug 
report at: https://github.com/espressif/arduino-esp32/issues/9465. And I changed the logger.cpp code to use 
log1.txt as a file name.

*/

#include "Logger.h"

extern const char* root_ca;   // Defined in secrets.h

LOGGER::LOGGER(){}

void LOGGER::info( String message )
{
  logit( "Info", message );
}

void LOGGER::warning( String message )
{
  logit( "Warning", message );
}

void LOGGER::error( String message )
{
  logit( "Error", message );
}

void LOGGER::critical( String message)
{
  logit( "Critical", message );
}

/*
* Adds host name and sends to active file
*/

void LOGGER::logit( String msgtype, String msg )
{  
  msg = msg.substring( 0, maxlogmsg - 1 );

  if ( echoSerial )
  {
    Serial.print( msgtype );
    Serial.print( F( ", " ) );
    Serial.println( msg );
  }

  if ( ! echoServer ) return;

  // Time to open a new log file?

  if ( mylogopen )
  {
    if ( mylog.size() > log_size_upload )
    {
      Serial.println( F( "Closing log file" ) );
      mylog.close();
      mylogopen = false;
    }
  }

  if ( ! mylogopen )
  {
    if ( ( ( millis() - logcreatepacetime ) < 500 ) )
    {
      return;
    }

    logcreatepacetime = millis();

    Serial.print( F("Starting new log file " ) );

    if ( ! scanLogNumbers() )
    {
      Serial.print( F("Logger scanLogNumbers failed" ) );
      return;
    }

    mylogname = LOGNAME_START;
    mylogname += String( highLogNumber + 1 );
    mylogname += LOGNAME_END;
    
    mylog = SD.open( mylogname, FILE_WRITE );
    if( ! mylog )
    {
      Serial.print( F( "Unable to create log file " ) );
      Serial.println( mylogname );
      return;
    }
    
    mylogopen = true;
    
    Serial.print( F( "Opened log file " ) );
    Serial.print( mylogname );
    Serial.print( ", mylog.size() = " );
    Serial.println( (long) mylog.size() );

    // I need to write something to the log file here because the 
    // Espressif SD library has a bug, it returns a random value for mylog.size() 
    // before something is written to the log file.

    mylog.print( devname.c_str() );
    mylog.print( F( ",Info," ) );
    mylog.print( F( "Starting log " ) );
    mylog.print( mylogname );
    mylog.print( F( "\n" ) );
    mylog.flush();
  }

  // Write to the log file

  long mysize = (long) mylog.size();

  String logmsg = devname.c_str();
  logmsg += ",";
  logmsg += msgtype;
  logmsg += ",";
  logmsg += msg;
  logmsg += "\n";

  int logval = mylog.print( logmsg );

  mylog.flush();

  // Then check the log file size is correct
  // Because the Espressif SD library has a nasty
  // bug where intermittently it will not actually
  // write to the file.

  if ( mylog.size() != ( mysize + logmsg.length() ) )
  {
    Serial.print( "Logger failed when checking " );
    Serial.print( mylogname );
    Serial.print( " size should be " );
    Serial.print( mysize + logmsg.length() );
    Serial.print( " instead it is " );
    Serial.print( mylog.size() );
    Serial.print( F( " forcecount = " ) );
    Serial.println( forcecount++ );

    Serial.println( F( "Closing log file" ) );
    mylog.close();
    mylogopen = false;
  }
}

/*

ChatGPT provided the urlencode() method from these prompts:
Are you familiar with URL encoding?
I am working in an ESP32 using Arduino IDE 2.3
I need a method in C to URL encode a String

ChatGPT details on the method:
This code defines a function urlencode that takes a null-terminated string as input and returns a 
dynamically allocated string containing the URL encoded version of the input string. It handles all 
characters in the string according to the URL encoding rules.

*/

char * LOGGER::urlencode(const char *str) 
{
    const char *hex = "0123456789ABCDEF";
    char *pstr = (char *)str, *buf, *pbuf;
    size_t len = strlen(str);
    size_t buf_len = len * 3 + 1; // Maximum possible length

    buf = (char *)malloc(buf_len);
    if (buf == NULL) {
        return NULL;
    }

    pbuf = buf;
    while (*pstr) {
        if ((*pstr >= 'A' && *pstr <= 'Z') ||
            (*pstr >= 'a' && *pstr <= 'z') ||
            (*pstr >= '0' && *pstr <= '9') ||
            *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') {
            *pbuf++ = *pstr;
        } else if (*pstr == ' ') {
            *pbuf++ = '+';
        } else {
            *pbuf++ = '%';
            *pbuf++ = hex[*pstr >> 4];
            *pbuf++ = hex[*pstr & 15];
        }
        pstr++;
    }
    *pbuf = '\0';

    return buf;
}

/*
* Sets log message echo to the Serial monitor
*/

void LOGGER::setEchoToSerial( bool echo )
{
  echoSerial = echo;
}

/*
* Sets log message echo to the log service in the Cloud
*/

void LOGGER::setEchoToServer( bool echo )
{
  echoServer = echo;
}

/*
* Sends message to the server over HTTPS GET protocol
*/

bool LOGGER::sendToServer( String message )
{
  //Serial.print( "sendToServer: ");
  //Serial.println( message );

  if ( message == "" )
  {
    //error( F("empty message") );
    return false;
  }

  char * encoded_str = urlencode( message.c_str() );
  if ( encoded_str == NULL )
  {
    error( F( "Failed to allocate memory for encoded string" ) );
    return false;
  } 

  String logurl = cloudCityLogURL;
  logurl += String( encoded_str );


  //Serial.println( "Sent to server:" );
  //Serial.println( logurl );


  http.begin( logurl, root_ca );
  http.setReuse(true);

  int httpCode = http.GET();

  if( httpCode != HTTP_CODE_OK )
  {
    Serial.print( F( "Server response code: " ) );
    Serial.println( httpCode );
    free(encoded_str);
    return false;
  }

  http.end();

  free(encoded_str);

  return true;
}

bool LOGGER::scanLogNumbers()
{
  lowLogNumber = 1000000;
  highLogNumber = 0;

  String mef = "/";
  mef += NAND_BASE_DIR;

  File root = SD.open( mef );
  if( !root )
  {
    info( F( "scanLogNumbers failed to open directory" ) );
    return false;
  }

  String mynum;
  int mynumbr = 0;

  for ( int j = 0; j < 1000; j++ )
  {
    File file = root.openNextFile();
    if ( file )
    {
      if ( ! file.isDirectory() )
      {
        String myfilename = file.name();

        if ( myfilename.startsWith( "log" ) )
        {
          int startPos = myfilename.indexOf("log") + 3;
          int endPos = myfilename.indexOf( LOGNAME_END );

          // Extract the numeric part of the filename
          String numericPart = myfilename.substring(startPos, endPos);

          // Convert the numeric part to an integer
          mynumbr = numericPart.toInt();

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
      return true;
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
  Serial.println( F( "Logger begin" ) );

  root_ca = ssl_cert;   // Defined in secrets.h

  echoSerial = false;   // Echo log messages to Serial monitor
  echoServer = true;    // Echo log messages to log service in the Cloud

  mylogopen = false;
  uploading = false;
  uploadcount = 1;
  forcecount = 0;
  deleting = 0;

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  devicename = devname.c_str();

  // Open a new log file

  if ( ! scanLogNumbers() ) return;

  mylogname = LOGNAME_START;
  mylogname += String( highLogNumber + 1 );
  mylogname += LOGNAME_END;

  mylog = SD.open( mylogname, FILE_WRITE );
  if( !mylog )
  {
    Serial.print( F( "Unable to create log file " ) );
    Serial.println( mylog );
  }
  
  mylogopen = true;
  
  Serial.print( F( "Opened starting log file " ) );
  Serial.print( mylogname );
  Serial.print( ", highLogNumber = " );
  Serial.print( highLogNumber );
  Serial.print( ", lowLogNumber = " );
  Serial.println( lowLogNumber );

  uploadchecktime = millis();
  uploadpacetime = millis();
  logcreatepacetime = millis();

  Serial.println( F( "Logger started" ) );
}

void LOGGER::loop()
{
  if ( ( ( millis() - uploadchecktime ) > 10000 ) )
  {
    uploadchecktime = millis();
    
    //Serial.println( F( "Checking for upload" ) );

    if ( ( ! uploading ) && ( ! deleting ) )
    {
      if ( ! scanLogNumbers() )
      {
        Serial.println( "Logger scanLogNumbers failed");
        return;
      }
      else
      {
        uploadfilename = LOGNAME_START;
        uploadfilename += String( lowLogNumber );
        uploadfilename += LOGNAME_END;

        if ( mylogopen ) 
        {
          if ( uploadfilename.equals( mylogname ) )
          {
            String mef = F( "Skipping log file because it is open " );
            mef += uploadfilename;
            Serial.println( mef );
            return;
          }
        }

        myupload = SD.open( uploadfilename );
        if( ! myupload )
        {
          Serial.print( F( "Logger unable to open upload file " ) );
          Serial.println( uploadfilename );

          uploading = false;
        }
        else
        {
          Serial.print( F( "Logger uploading " ) );
          Serial.println( uploadfilename );

          uploading = true;
        }
      }
    }
  }

  if ( ( uploading ) && ( ! deleting ) )
  {
    if ( ( ( millis() - uploadpacetime ) > 500 ) )
    {
      uploadpacetime = millis();

      //uploadstr = myupload.readStringUntil('\n');
      //if ( uploadstr.length() > 0 )


      bytesRead = myupload.readBytes(buffer, 100);
      if ( bytesRead > 0 )

      {  
        data = String( buffer );
        Serial.print( "   Simulation: " );
        Serial.println( data );
        //Serial.println( uploadstr );
        // sendToServer( uploadstr );
      }
      else
      {
        Serial.print( F( "Logger sent " ) );
        Serial.print( uploadfilename );
        Serial.println( F( " to service" ) );

        myupload.close();
        uploading = false;
        uploadcount++;
        deleting = true;
        deletetime = millis();
      }
    }
  }

  if ( deleting )
  {
    if ( ( ( millis() - deletetime ) > 2000 ) )
    {
      deleting = false;

      if ( SD.remove( uploadfilename ) )
      {
        Serial.println( F("Log file deleted successfully") );
      }
      else
      {
        Serial.println( F( "Log file delete failed" ) );
      }
    }
  }
}
