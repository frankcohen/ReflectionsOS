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

  if ( echoServer )
  {
    String logmsg = devname.c_str();
    logmsg += ",";
    logmsg += msgtype;
    logmsg += ",";
    logmsg += msg;
    logmsg += "\n";

    appendToBuffer( logmsg );
  }
}

// Append data to the buffer

void LOGGER::appendToBuffer( String data )
{
  int dataLength = data.length();
  if ( bufferIndex + dataLength < log_max_size )
  {
    // Append data to buffer
    data.toCharArray( buffer + bufferIndex, dataLength + 1 );
    bufferIndex += dataLength;
  }
  else
  {
    // Buffer overflow, write buffer contents to log file
    writeBufferToFile();

    // Reset buffer and append data again
    data.toCharArray( buffer, dataLength + 1 );
    bufferIndex = dataLength;
  }
}

// Write buffer contents to log file

void LOGGER::writeBufferToFile() 
{
  int mylognum = 0;

  if ( scanLogNumbers() )
  {
    mylognum = highLogNumber + 1;
  }
  
  mylogname = LOGNAME_START;
  mylogname += String( mylognum );
  mylogname += LOGNAME_END;

  File logFile = SD.open( mylogname, FILE_WRITE ); // Open the log file for writing
  if ( logFile ) 
  {
    logFile.write( (const uint8_t *) buffer, bufferIndex );
    logFile.flush();
    logFile.close();
    bufferIndex = 0; // Reset buffer index
    Serial.println("Buffer written to log file.");
  }
  else
  {
    Serial.println("Error writing to log file");
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

Here is how you use it:

  char * encoded_str = urlencode( message.c_str() );
  if ( encoded_str == NULL )
  {
    error( F( "Failed to allocate memory for encoded string" ) );
    return false;
  } 

  String logurl = cloudCityLogURL;
  logurl += String( encoded_str );

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
* Sends log to the server over HTTPS Post protocol
*/

bool LOGGER::sendToServer( String logfilename )
{
  File logFile = SD.open( logfilename );
  if ( !logFile ) 
  {
    Serial.print( F( "Error opening log file ") );
    Serial.println( logfilename );
    return false;
  }

  String logData = "";

  while ( logFile.available() ) 
  {
    logData += (char) logFile.read();
  }

  logFile.close();

  HTTPClient http;

  http.begin( cloudCityLogPostURL, root_ca );
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.POST( logData );

  if (httpResponseCode > 0) 
  {
    Serial.printf( "Logger sendToServer POST... response code: %d\n", httpResponseCode );
    if ( httpResponseCode == HTTP_CODE_OK ) 
    {
      return true;
    }
  } 
  else
  {
    Serial.printf( "Logger sendToServer POST... failed, response error: %s\n", http.errorToString(httpResponseCode).c_str() );
  }

  http.end();

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
      file.close();
    }
  }

  if ( ( lowLogNumber == 1000000 ) || ( highLogNumber == 0 ) )
  {
    return false;
  }
  else
  {
    return true;
  }
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

  int bufferIndex = 0;
 
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
    
    if ( ! scanLogNumbers() )
    {
      //Serial.println( "Logger scanLogNumbers failed");
    }
    else
    {
      uploadfilename = LOGNAME_START;
      uploadfilename += String( lowLogNumber );
      uploadfilename += LOGNAME_END;

      uploading = true;

      if ( sendToServer( uploadfilename ) )
      {
        Serial.print( F( "Logger sent " ) );
        Serial.print( uploadfilename );
        Serial.println( F( " to service" ) );

        uploading = false;
        uploadcount++;

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
}
