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

#include "Player.h"

// Defined in ReflectionsOfFrank.ino
extern LOGGER logger;
extern Video video;
extern Audio audio;
extern Storage storage;

Player::Player() {}

void Player::RetreatGesture()
{}

void Player::SkipGesture()
{}

void Player::PauseGesture()
{}

void Player::DeleteGesture()
{}

// Additional gesture ideas
// Fast forward
// Fast reverse

/* Shows menu images from archives (ffmpeg recorder), in order of most played */

void Player::ShowMenu()
{
}

bool Player::tarsExist()
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  File dirIter = SD.open( mef.c_str() );

  if( !dirIter )
  {
    logger.info( F( "Player failed to open directory / finding tars" ) );
    return false;
  }

  File file = dirIter.openNextFile();
  if ( ! file )
  {
    return false;
  }
  else
  {
    if( ! file.isDirectory() )
    {
      String myname = file.name();
      if ( myname.indexOf( ".tar" ) > 0 )
      {
        return true;
      }
    }
  }
  return false;
}

bool Player::startAtTop()
{
  String mef = "/";
  mef += NAND_BASE_DIR;

  showDirectoryIterator = SD.open( mef.c_str() );
  if( !showDirectoryIterator )
  {
    logger.error( F( "Player startAtTop failed to open" ) );
    return false;
  }
  return true;
}

bool Player::findNext()
{ 
  File file = showDirectoryIterator.openNextFile();
  if ( ! file )
  {
    String mef = "/";
    mef += NAND_BASE_DIR;
    showDirectoryIterator = SD.open( mef.c_str() );
    if( !showDirectoryIterator )
    {
      logger.error( F( "Player failed to open directory /" ) );
      startAtTop();
      return false;
    }
  }

  if( ! file.isDirectory() )
  {
    return false;
  }
  else
  {
    nextDir = file.path();

    String msg = "Player findNext nextDir: ";
    msg += nextDir;
    logger.info( msg );

    if ( nextDir.startsWith("/.") )
    {
      String msg = "Player findNext skipping directory: ";
      msg += nextDir;
      logger.info( msg );

      return false;
    }

    return decodeShow( nextDir );
  }
  return true;
}

bool Player::decodeShow( String showName )
{
  String sc = NAND_BASE_DIR;
  String lilname = showName.substring( sc.length() + 2 );

  String script = "/";
  script += NAND_BASE_DIR;
  script += "/";
  script += lilname;
  script += "/";
  script += lilname;
  script += ".json";
  
  File scriptFile = SD.open( script );

  if ( ! scriptFile )
  {
    Serial.print( F( "Player, file not found: " ));
    Serial.println( script );
    return false;
  }

  /* Fixme later: JSON data size limited */
  DynamicJsonDocument doc(500);

  Serial.print( F( "Shows: deserialize " ) );
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
      nextDir = showName;
      
      Serial.print( "nextVideo: " );
      Serial.println( nextVideo );
      Serial.print( "nextAudio: " );
      Serial.println( nextAudio );

      return true;
    }
  }

  return true;
}

void Player::play( String mname )
{
  if ( video.getStatus() > 0 )
  {
    video.stopVideo();
  }

  // decode mjpeg and audio from show json instructions

  if ( decodeShow( mname ) )
  {
    playerStatus = 1;
    String mef = "/";
    mef += NAND_BASE_DIR;
    mef += "/" + nextDir + "/" + nextVideo;
    video.startVideo( mef );

    mef = "/";
    mef += NAND_BASE_DIR;
    mef += "/" + nextDir + "/" + nextAudio;
    audio.play( mef );
  }
  else
  {
    logger.error( F( "Player play decodeShow returned false" ) );
  }
}

void Player::begin()
{
  playerStatus = 0;
  playertime = millis();
  startAtTop();
  checktime = millis();
}

void Player::loop()
{
  if ( video.getStatus() > 0 ) return;

  if ( ( millis() - playertime ) > ( 2 * 60000 ) )
  {
    logger.info( F( "Player replicating" ) );
    //storage.replicateServerFiles();
    //startAtTop();

    playertime = millis();
  }

  if ( ( millis() - checktime ) > 5000 )
  {
    checktime = millis();

    if ( findNext() )
    {
      playerStatus = 1;

      String mef = nextDir + "/" + nextVideo;
      video.startVideo( mef );
      logger.info( "video startVideo " + mef );

      mef = nextDir + "/" + nextAudio;
      audio.play( mef );
      logger.info( "audio play " + mef );
    }
  }

}
