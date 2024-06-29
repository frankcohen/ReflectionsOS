/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for operating the video display.

*/

#include "Video.h"

// Defined in ReflectionsOfFrank.ino
extern LOGGER logger;
extern TOF tof;

static MjpegClass mjpeg;
uint8_t *mjpeg_buf;

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 1 /* rotation */, false /* IPS */);
Arduino_GFX *bufgfx = new Arduino_Canvas( 240 /* width */, 240 /* height */, gfx );

#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame

/* pixel drawing callback */

static int jpegDrawCallback( JPEGDRAW *pDraw )
{
  //Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  // output_display -> flush();
  
  return 1;
}

Video::Video() {}

void Video::begin()
{
  #ifdef GFX_EXTRA_PRE_INIT
    GFX_EXTRA_PRE_INIT();
  #endif

  mjpeg_buf = (uint8_t *) malloc( MJPEG_BUFFER_SIZE );
  if ( !mjpeg_buf )
  {
    Serial.println( F("mjpeg_buf malloc failed, stopping" ) );
    stopOnError( "Video buffer", "fail", "", "", "" );
  }

  gfx->begin();
  gfx->invertDisplay(true);
  gfx->fillScreen( COLOR_BLACK );

  videoStatus = 0;   // idle
  firsttime = true;
  vidtimer = millis();
  paused = false;

  playerStatus = 0;
  checktime = millis();
  showIteratorFlag = false;
}

void Video::addReadTime( unsigned long rtime )
{
  totalReadVideo += rtime;
}

/* Show error on display, then halt */

void Video::stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 )
{
  bufgfx->begin();
  gfx->invertDisplay(true);
  gfx->fillScreen( COLOR_BACKGROUND );
  bufgfx->invertDisplay(true);
  bufgfx->fillScreen( COLOR_BACKGROUND );

  String errmsg = "Video stopOnError ";
  errmsg += msg1;
  errmsg += ", ";
  errmsg += msg2;
  errmsg += ", ";
  errmsg += msg3;
  errmsg += ", ";
  errmsg += msg4;
  errmsg += ", ";
  errmsg += msg5;
  logger.error( errmsg );

  ringtimer = millis();

  while(1)
  {
    int16_t diam = 20;
    int16_t x = random( 40, 200 );
    int16_t y = random( 40, 200 );

    bufgfx -> drawCircle( x, y, diam, COLOR_BACKGROUND);
    bufgfx -> drawCircle( x, y, diam - 1, COLOR_BACKGROUND);
    bufgfx -> drawCircle( x, y, diam - 2, COLOR_LEADING);

    bufgfx -> drawCircle( x, y, diam - 3, COLOR_RING);
    bufgfx -> drawCircle( x, y, diam - 4, COLOR_TRAILING);

    bufgfx -> fillRect( 40, 40, 160, 160, COLOR_TEXT_BACKGROUND );
    bufgfx -> drawRect( 39, 39, 162, 162, COLOR_TEXT_BORDER );
    bufgfx -> drawRect( 40, 40, 160, 160, COLOR_TEXT_BORDER );

    bufgfx->setFont(&FreeSansBold10pt7b);
    bufgfx->setTextColor( COLOR_LEADING );
    bufgfx->setCursor( leftmargin, topmargin - 5 );
    bufgfx->println("REFLECTIONS");

    bufgfx->setCursor( leftmargin, topmargin + ( 1 * linespacing ) );
    bufgfx->setFont(&FreeSerif8pt7b);
    bufgfx->setTextColor( COLOR_TEXT );
    bufgfx->println( msg1 );

    bufgfx->setCursor( leftmargin, topmargin + ( 2 * linespacing ) );
    bufgfx->setTextColor( COLOR_TEXT );
    //bufgfx->setFont(&FreeSerifBoldItalic12pt7b);
    bufgfx->println( msg2 );

    bufgfx->setCursor( leftmargin, topmargin + ( 3 * linespacing ) );
    bufgfx->println( msg3 );

    bufgfx->setCursor( leftmargin, topmargin + ( 4 * linespacing ) );
    bufgfx->println( msg4 );

    bufgfx->setCursor( leftmargin, topmargin + ( 5 * linespacing ) );
    bufgfx->println( msg5 );

    bufgfx -> flush();

    delay(500);
  }
}

/* Uses showIteratorFlag to scan through the subdirectories in /REFLECTIONS/
   Subdirectories contain a manifest file in json format. The manifest identifies
   the mjpeg and audio file to play.
   Note: this was previously used to play multiple videos in sequence
*/
   
bool Video::findNextVideo()
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

// Returns milliseconds since the video started playing

unsigned long Video::getVideoTime()
{
  return millis() - videoStartTime;
}

// Utility to scan /REFLECTIONS directory for tar files

bool Video::tarsExist()
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

void Video::resetStats()
{
  totalFrames = 0;
  totalReadVideo = 0;
  totalDecodeVideo = 0;
  totalShowVideo = 0; 
  startMs = millis();
}

int Video::getStatus()
{
  return videoStatus;
}

/* Play a .mjpeg file to the display */

void Video::startVideo( String vname )
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += vname;
  mef += "/";
  mef += vname;
  mef += videoname_end;

  String msg = "startVideo ";
  msg += mef;
  logger.info( msg );

  mjpegFile = SD.open( mef );
  if ( ! mjpegFile )
  {
    videoStatus = 0;
    String msg = "startVideo failed to open ";
    msg += mef;
    logger.error( msg );
    return;
  }

  if ( mjpeg.setup(
    &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
    0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */, firsttime ) )
  {
    logger.info( F( "Mjpeg is set-up" ) );
    firsttime = false;
    videoStatus = 1;
  }
  else
  {
    Serial.println( F( "Could not set-up mjpeg") );
    videoStatus = 0;
  }

  videoStartTime = millis();
}

void Video::stopVideo()
{
  mjpegFile.close();
  videoStatus = 0;
}

void Video::setPaused( bool p )
{
  paused = p;
}

void Video::setTofEyes( bool status )
{
  tofEyes = status;
}

#define smallcircle 20
#define bigcircle 40
#define maxy 40
#define bridgex 45

void Video::drawTofEyes()
{ 
  int basey = 40;

  int basex = tof.getXFingerPosition();
  //basex += pointx * ( ( 240 - 30 - 30 - smallcircle ) / 8 );

  int topx = basex;
  topx += bigcircle / 2;
  int topy = basey;
  topy += bigcircle / 2;

  int middlex = topx;
  int middley = topy + ( bigcircle / 2 );

  int bottomx = middlex;
  int bottomy = middley + ( bigcircle / 2 );

  gfx -> fillRect( 0, maxy, 240, maxy * 2, COLOR_LEADING );

  gfx -> fillCircle( topx, topy, smallcircle /2, COLOR_RING );
  gfx -> fillCircle( middlex, middley, bigcircle / 2, COLOR_RING );
  gfx -> fillCircle( bottomx, bottomy, smallcircle /2, COLOR_RING );

  gfx -> fillCircle( topx + bridgex, topy, smallcircle /2, COLOR_RING );
  gfx -> fillCircle( middlex + bridgex, middley, bigcircle / 2, COLOR_RING );
  gfx -> fillCircle( bottomx + bridgex, bottomy, smallcircle /2, COLOR_RING );
}

void Video::printCentered( int y2, String text, uint16_t color, const GFXfont * font )
{
  gfx->setFont( font );

  int16_t x1, y1;
  uint16_t w, h;

  gfx->getTextBounds( text, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (gfx->width() - w) / 2;
  //int16_t y = (gfx->height() - h) / 2;
  //y += y2;

  gfx->setCursor(x, y2);
  gfx->setTextColor( color );
  //gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, YELLOW);
  gfx->println( text );
}

void Video::loop()
{
  if ( ( videoStatus == 0 ) || ( paused == 1 ) ) return;

  if ( mjpegFile.available() )
  {
    if ( (millis() - vidtimer ) > 50 ) 
    {
      vidtimer = millis();

      totalFrames++;

      unsigned long dtime = millis();

      if ( ! mjpeg.readMjpegBuf() )
      {
        //logger.error( F("readMjpegBuf returned false") );
        stopVideo();
        return;
      }

      totalDecodeVideo += millis() - dtime;
      dtime = millis();

      if ( tofEyes && tof.tofStatus() )
      {
        drawTofEyes();
      }
      else
      {
        mjpeg.drawJpg();
      }

      totalShowVideo += millis() - dtime;
    }
  }
  else
  {
    vidtimer = millis();
    stopVideo();

    totalTime = millis() - startMs;

    String stats = "Video stats, ";
    stats += "Total frames ";
    stats += totalFrames;
    stats += ", time used ";
    stats += totalTime;

    stats += ", totalReadVideo ";
    stats += totalReadVideo;
    stats += ", totalDecodeVideo ";
    stats += totalDecodeVideo;
    stats += ", totalShowVideo ";
    stats += totalShowVideo;
    
    stats += ", read ";
    stats += ( 100.0 * ( totalReadVideo / totalTime ) );
    stats += "%, decode ";
    stats += ( 100.0 * ( totalDecodeVideo / totalTime ) );
    stats += "%, show ";
    stats += ( 100.0 * ( totalShowVideo / totalTime ) );
    stats += "%, fps ";
    stats += ( 1000.0 * ( totalFrames / totalTime ) );
    logger.info( stats );
  }

  /*

  if ( getStatus() == 1 )
  {
    if ( ( millis() - checktime ) > 2000 )
    {
      checktime = millis();

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
      paused = false;
      logger.info( "video startVideo " + mef );

      //mef = nextDir + "/" + nextAudio;
      //audio.play( mef );
      //logger.info( "audio play " + mef );
      
    }
    else
    {
    }
  }
*/

}
