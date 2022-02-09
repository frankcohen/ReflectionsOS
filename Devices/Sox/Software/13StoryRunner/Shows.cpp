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
  sequence = 1;
  showDirectoryIterator = SD.open("/");
  showTimer = millis() + 1000;
  dirTimer = millis() + 3000;
}

void Shows::loop()
{
  if ( millis() > showTimer )
  {
    showTimer = millis() + 10000;
  }
}

String Shows::getNextVideo()
{
  return nextDir + "/" + nextVideo;
}

String Shows::getNextAudio()
{
  return nextDir + "/" + nextAudio;
}

boolean Shows::findNext()
{
  File file = showDirectoryIterator.openNextFile();
  if ( ! file )
  {
    if ( millis() > dirTimer )
    {
      dirTimer = millis() + 10000;
      Serial.println( "Shows findNext did not find a file/directory. Starting at top." );
      showDirectoryIterator = SD.open("/");
      return false;
    }
  }
  else
  {
    if( ! file.isDirectory() )
    {
      Serial.print( "Shows findNext Ignoring file ");
      Serial.println( file.name() );
      return false;
    }
    else
    {
      nextDir = file.name();
      Serial.print( "nextDir: ");
      Serial.println( nextDir );

      String myname = file.name();
      String script = myname + myname + ".json";
      File scriptFile = SD.open( script );

      if ( ! scriptFile )
      {
        Serial.print( "Shows, file not found: ");
        Serial.println( scriptFile );
        return false;
      }

      /* Fixme later: JSON data size limited */
      DynamicJsonDocument doc(500);

      Serial.print( "Shows, deserialize ");
      Serial.println( scriptFile.name() );

      DeserializationError error = deserializeJson(doc, scriptFile );
      if (error) {
        Serial.print(F("Shows, deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
      }

      // serializeJsonPretty(doc, Serial);

      String thevideofile;
      String theaudiofile;

      JsonObject eventseq = doc["ReflectionsShow"]["events"];
      //serializeJsonPretty(eventseq, Serial);

      for (JsonObject::iterator it = eventseq.begin(); it!=eventseq.end(); ++it)
      {
        String itkey = it->key().c_str();
        String itvalue = it->value().as<char*>();

        //Serial.print( "itkey: ");
        //Serial.println( itkey );

        JsonArray sequence = doc["ReflectionsShow"]["events"][itkey]["sequence"];
        for (JsonObject step : sequence)
        {
          String nv = step["playvideo"];
          nextVideo = nv;
          String na = step["playaudio"];
          nextAudio = na;
          readyForNextMedia = true;

          Serial.print( "nextVideo: " );
          Serial.println( nextVideo );
          Serial.print( "nextAudio: " );
          Serial.println( nextAudio );

          return true;
        }
      }
    }
  }
  return true;
}

boolean Shows::getReadyForNextMedia()
{
  return readyForNextMedia;
}

void Shows::setReadyForNextMedia( boolean nm )
{
  readyForNextMedia = nm;
}

boolean Shows::getReadyForNextShow()
{
  return readyForNextShow;
}

void Shows::setReadyForNextShow( boolean rfns )
{
  readyForNextShow = rfns;
}
