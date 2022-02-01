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

This file is for Shows tasks, running shows, stepping through shows

*/

#include "Arduino.h"
#include "Storage.h"
#include <ArduinoJson.h>

#include "Shows.h"

Shows::Shows(){}

void Shows::begin()
{
  readyForNextShow = true;
}

void Shows::loop()
{
  /* Fixme later: JSON data size limited */
  DynamicJsonDocument doc(500);

  if ( readyForNextShow )
  {
    // Find the first show (it's a directory, with json contents )

    if ( showDirectoryIterator == NULL )
    {
      showDirectoryIterator = SD.open("/");
      if( ! showDirectory )
      {
        Serial.println( "Error on opening showDirectory" );
        return;
      }

      if ( ! ( String( show.name()).endsWith( JSON_FILENAME ) ) )
      {
        Serial.printf_P(PSTR("Skipping non-show file '%s'\n"), show.name());
        return;
      }
      else
      {
        readyForNextShow = false;

        DeserializationError error = deserializeJson(doc, show );
        if (error) {
          Serial.print(F("Show deserializeJson() failed: "));
          Serial.println(error.c_str());
          return;
        }

        // serializeJsonPretty(doc, Serial);

        JsonObject fileseq = doc["ReflectionsShow"];

        for (JsonObject::iterator it = fileseq.begin(); it!=fileseq.end(); ++it)
        {
          String itkey = it->key().c_str();
          String itvalue = it->value().as<char*>();

          Serial.println( itkey );

          JsonObject group = doc["ReflectionsShow"]["events"][ itkey ];

          for (JsonObject::iterator groupit = group.begin(); groupit!=group.end(); ++groupit)
          {
            String groupkey = groupit->key().c_str();
            String groupvalue = groupit->value().as<char*>();

            Serial.print( "groupkey " );
            Serial.print( groupkey );
            Serial.print( " groupvalue " );
            Serial.println( groupvalue );

            //if ( groupkey.equals( "file" ) ) { thefile = groupvalue; }
          }
        }
      }
    }
  }
}

// Parse the show, set-up the show state machine

void Shows::openShow()
{

}

void Shows::getReadyForNextShow( boolean rfns )
{
  readyForNextShow = rfns;
}
