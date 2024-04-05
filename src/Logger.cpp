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

  if ( ! mylogopen )
  {
    Serial.println( F( "Not logging, mylog is not open" ) );
    return;    
  }

  String logmsg = devname.c_str();
  logmsg += ",";
  logmsg += msgtype;
  logmsg += ",";
  logmsg += msg;
  logmsg += "\n";

  long mysize = (long) mylog.size();

  int logval = mylog.print( logmsg );
  mylog.flush();

  /*
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
  */

  if ( mylog.size() == mysize + logmsg.length() )
  {
    //Serial.print( F( ", forcecount = " ) );
    //Serial.print( forcecount );
    //Serial.println( F( " OK" ));
    
    forceit = true;
  }
  else
  {
    //Serial.println( ", FAIL" );

    Serial.print( F("Starting new log file, forcecount = " ) );
    Serial.println( forcecount++ );
    forceit = false;
  }

  if ( forceit )
  {
    if ( mylog.size() < log_size_upload ) return;

    if ( ! scanLogNumbers() ) return;
  }

  mylog.close();
  mylogopen = false;

  String elname = LOGNAME_START;
  elname += String( highLogNumber + 1 );
  
  lognext = SD.open( elname, FILE_WRITE );
  if( !lognext )
  {
    Serial.print( F( "Unable to create log file " ) );
    Serial.println( elname );
    return;
  }
  
  mylog = lognext;
  mylogname = elname;
  mylogopen = true;
  
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
  Serial.print( "sendToServer: ");
  Serial.println( message );

  if ( message == "" )
  {
    error( F("empty message") );
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


  Serial.println( "Sent to server:" );
  Serial.println( logurl );


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
          mynum = myfilename.substring( 3 );
          mynumbr = mynum.toInt();

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
  myuploadopen = false;
  uploading = false;
  uploadcount = 1;
  forceit = false;
  forcecount = 0;

  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  devicename = devname.c_str();

  // Open a new log file

  if ( ! scanLogNumbers() ) return;

  mylogname = LOGNAME_START;
  mylogname += String( highLogNumber + 1 );
  
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

  Serial.println( F( "Logger started" ) );
}

void LOGGER::loop()
{
  if ( ( ( millis() - uploadchecktime ) > 10000 ) )
  {
    uploadchecktime = millis();
    
    //Serial.println( F( "Checking for upload" ) );

    if ( !uploading )
    {
      if ( scanLogNumbers() )
      {
        uploadfilename = LOGNAME_START;
        uploadfilename += String( lowLogNumber );

        if ( ( mylogopen ) && ( uploadfilename.equals( mylogname ) ) )
        {
          String mef = F( "Skipping log file because it is open " );
          mef += uploadfilename;
          info( mef );
        }
        else
        {
          myupload = SD.open( uploadfilename );
          if( !myupload )
          {
            String mef = F( "Logger unable to open upload file " );
            mef += uploadfilename;
            info( mef );

            myuploadopen = true;
            uploading = false;
          }
          else
          {
            String mef = F( "Logger uploading log " );
            mef += uploadfilename;
            info( mef );

            uploading = true;
            myuploadopen = true;
          }
        }
      }
    }
    else
    {
      info( F( "Logger uploading in progress" ) );
    }
  }

  if ( uploading )
  {
    long avail = myupload.available();

    if ( avail > 0 )
    {
      String mys = myupload.readStringUntil('\n');
      sendToServer( mys );
    }
    else
    {
      String mef = F( "Logger sent " );
      mef += uploadfilename;
      mef += " to service and deleted the file ";
      mef += uploadcount;
      info( mef );

      uploadcount++;
      myupload.close();
      myuploadopen = false;
      SD.remove( uploadfilename );
      uploading = false;
    }
  }
}
