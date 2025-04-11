/*
 Reflections, mobile connected entertainment experience

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

*/

#include "Logger.h"

LOGGER::LOGGER(){}

void LOGGER::begin()
{
}

void LOGGER::info( String message )
{
  logit( F("Info"), message );
}

void LOGGER::warning( String message )
{
  logit( F("Warning"), message );
}

void LOGGER::error( String message )
{
  logit( F("Error"), message );
}

void LOGGER::critical( String message)
{
  logit( F("Critical"), message );
}

/*
* Adds host name and sends to active file
*/

void LOGGER::logit( String msgtype, String msg )
{  
    Serial.print( msgtype );
    Serial.print( F( ", " ) );
    Serial.println( msg );
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
