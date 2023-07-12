/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on JPEGDEC: https://github.com/bitbank2/JPEGDEC.git

 This file is for Shows tasks, running shows, stepping through shows
*/

#include "Shows.h"

Shows::Shows() {}

void Shows::begin()
{
  readyForNextShow = true;
  sequence = 1;
  showTimer = millis() + 1000;
  dirTimer = millis() + 3000;

  fileValid = false;

  showDirectoryIterator = SD.open("/");
  if(!showDirectoryIterator){
      Serial.println( F( "Shows begin: Failed to open directory /" ) );
      return;
  }

  fileValid = true;
}

void Shows::loop()
{
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
  if ( ! fileValid ) return false;
  
  File file = showDirectoryIterator.openNextFile();
  if ( ! file )
  {
    if ( millis() > dirTimer )
    {
      dirTimer = millis() + 10000;
      Serial.println( "Starting at top." );

      showDirectoryIterator = SD.open("/");
      if(!showDirectoryIterator){
          Serial.println( F( "Shows: Failed to open directory /" ));
          fileValid = false;
          return false;
      }
      return false;
    }    
  }
  else
  {
    if( ! file.isDirectory() )
    {
      Serial.print( F( "Shows findNext Ignoring file " ) );
      Serial.println( file.name() );
      return false;
    }
    else
    {
      nextDir = file.path();
      Serial.print( F( "nextDir: " ) );
      Serial.println( nextDir );

      if ( nextDir.startsWith("/.") )
      {
        Serial.print( F( "Skipping directory starting with a period " ) );
        Serial.println( nextDir );
        return false;
      }

      String myname = file.path();
      String script = myname + "/" + myname + ".json";
      File scriptFile = SD.open( script );

      if ( ! scriptFile )
      {
        Serial.print( F( "Shows, file not found: " ));
        Serial.println( scriptFile );
        return false;
      }

      /* Fixme later: JSON data size limited */
      DynamicJsonDocument doc(500);

      Serial.print( F( "Shows, deserialize " ) );
      Serial.println( scriptFile.name() );

      DeserializationError error = deserializeJson(doc, scriptFile );
      if (error) {
        Serial.print(F("Shows, deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
      }

      serializeJsonPretty(doc, Serial);

      String thevideofile;
      String theaudiofile;

      JsonObject eventseq = doc["ReflectionsShow"]["events"];
      //serializeJsonPretty(eventseq, Serial);

      for (JsonObject::iterator it = eventseq.begin(); it!=eventseq.end(); ++it)
      {
        String itkey = it->key().c_str();
        String itvalue = it->value().as<const char*>();

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
