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

  if ( mylogopen != 1 )
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
        
    forceit = 1;
  }
  else
  {
    //Serial.println( ", FAIL" );

    Serial.print( F("Starting new log file, forcecount = " ) );
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
  Serial.println( F( "Logger begin" ) );

  root_ca = ssl_cert;   // Defined in secrets.h

  echoSerial = false;   // Echo's log messages to Serial monitor
  echoServer = true;    // Echo's log messages to log service in the Cloud

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
        uploadfilename = logname_start;
        uploadfilename += String( lowLogNumber );

        if ( ( mylogopen ) && ( uploadfilename.equals( mylogname ) ) )
        {
          //Serial.print( F( "Skipping log file because it is open " ) );
          //Serial.println( uploadfilename );
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
            
            Serial.print( F( "Uploading log " ) );
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
      char line[ maxlogmsg + 1 ];
      for ( int j = 0; j < maxlogmsg; j++)
      {
        line[j] = 0;
      }
      char fchr;

      int i=0;
      while ( ( i < maxlogmsg ) && myupload.available() )
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
      //Serial.print( F( "Log sent to service, count = " ) );
      //Serial.println( uploadcount );

      myupload.close();
      myuploadopen = 0;
      SD.remove( uploadfilename );
      uploading = 0;
    }
  }
}
