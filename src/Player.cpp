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
extern Accelerometer accel;

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

bool Player::findNext()
{ 
  if ( ! showIteratorFlag )
  {
    String mef = "/";
    mef += NAND_BASE_DIR;

    showDirectoryIterator = SD.open( mef.c_str() );
    if( ! showDirectoryIterator )
    {
      showIteratorFlag = false;
      logger.error( F( "Player failed to open directory iterator" ) );
      return false;
    }

    showIteratorFlag = true;
  }

  findMore = true;
  twice = 0;
  File file;

  while ( ( findMore ) && ( twice < 2 ) )
  {
    file = showDirectoryIterator.openNextFile();
    if ( ! file )
    {
      twice++;

      String mef = "/";
      mef += NAND_BASE_DIR;

      showDirectoryIterator = SD.open( mef.c_str() );
      if( ! showDirectoryIterator )
      {
        logger.error( F( "Player, showDirectoryIterator failed" ) );
        showDirectoryIterator.close();
        showIteratorFlag = false;
      }
    }

    if( file.isDirectory() )
    {
      findMore = false;
    }
  }

  if ( twice >= 2 )
  {
    logger.error( F( "Player, showDirectoryIterator failed" ) );
    return false;
  }

  String showName = file.path();

  String msg = "Player findNext nextDir is ";
  msg += showName;
  logger.info( msg );

  if ( nextDir.startsWith("/.") )
  {
    String msg = "Player findNext skipping directory ";
    msg += showName;
    logger.info( msg );
    return false;
  }

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
    String mef = F( "Player, file not found: " );
    mef += script;
    logger.info( mef );
    return false;
  }

  /* Fixme later: JSON data size limited */
  DynamicJsonDocument doc(500);

  //Serial.print( F( "Shows: deserialize " ) );
  //Serial.println( scriptFile.name() );

  DeserializationError error = deserializeJson(doc, scriptFile );
  if (error) {
    Serial.print(F("Shows, deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }

  //serializeJsonPretty(doc, Serial);

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

      /*
      String mef = F( "Player, nextVideo ");
      mef += nextVideo;
      logger.info( mef );
      String mef2 = F( "Player, nextAudio ");
      mef2 += nextAudio;
      logger.info( mef2 );
      */

      return true;
    }
  }

  return false;
}

void Player::begin()
{
  playerStatus = 0;
  checktime = millis();
  showIteratorFlag = false;
}

void Player::loop()
{
  if ( video.getStatus() == 1 )
  {
    if ( ( millis() - checktime ) > 2000 )
    {
      checktime = millis();

      int ges = accel.getRecentGesture();

      if ( ges == 0 ) return;

      // Next video
      if ( ges > 0 )
      {
        video.stopVideo();

        if ( findNext() )
        {
          playerStatus = 1;
          logger.info( F( "Player next video" ) );

          //mef = nextDir + "/" + nextAudio;
          //audio.play( mef );
          //logger.info( "audio play " + mef );
          
        }
      }

      // Previous video
      if ( ges == 2 )
      {
        
      }
    }

    return;
  }
  
  if ( ( millis() - checktime ) > 2000 )
  {
    checktime = millis();

    if ( findNext() )
    {
      playerStatus = 1;

      String mef = nextDir + "/" + nextVideo;
      video.startVideo( mef );
      logger.info( "video startVideo " + mef );

      //mef = nextDir + "/" + nextAudio;
      //audio.play( mef );
      //logger.info( "audio play " + mef );
      
    }
    else
    {
    }
  }

}
