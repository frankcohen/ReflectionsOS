/*
 * Decodes a Show file (JSON format) and runs on the Seuss Board
 *
 * Board wiring directions and code at https://github.com/frankcohen/ReflectionsOS
 *
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * February 14, 2021 Happy Valentines Day

   Using Arduino_GFX class by @moononournation
   https://github.com/moononournation/Arduino_GFX
   For speed and support of MPEG video

   Note: To play 240x240 MJPEG uncompressed files requires the audio at
   a sample rate of 8000 kHz, 16 bits, stereo format, WAVE format

   Using ESP32-targz library to uncompress tar files
   https://github.com/tobozo/ESP32-targz

   Using Arduino_JSON 0.1.0 Beta https://github.com/bblanchon/ArduinoJson

 */

// The Basics
#include <WiFi.h>
#include <WiFiMulti.h>
#include "HTTPClient.h"
WiFiMulti WiFiMulti;

// Franks_Free_Internet_Guest, no password
// FrankCohenCleverMoe, thanksfrank
// chickencoop, chickens

#define NETWORK "836CAE"
#define PASSWORD "12C26C2C50191"

// SD card and file system
#include <SD.h>
boolean sdCardValid = false;
#define SD_CS_PIN 4

// Backlight
#include <FastLED.h>
#define DATA_PIN    33
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];
#define BRIGHTNESS  250   // Backlight brightness using WS2812 Neopixel LED units

// Cloud City
#define serverDomain "54.190.177.23"
#define serverPort 80

// Seuss Display
#define SCK 18
#define MOSI 23
#define MISO 19
#define SS 4
#define MJPEG_FILENAME ".mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)
#include <Arduino_GFX_Library.h>
#define DisplayCS 32     // TFT display on Adafruit's ST7789 card
#define DisplaySDCS 4    // SD card on the Adafruit ST7789 card
#define DisplayRST 17    // Reset for Adafruit's ST7789 card
#define DisplayDC 16     // DC for Adafruit's ST7789 card
Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
boolean firstTime = true;
uint8_t *mjpeg_buf;

// Movie stream library
#include "MjpegClass.h"
static MjpegClass mjpeg;

// I2S for Audio
#define bclk 26
#define wclk 25
#define dout 27
#include <FS.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
File audiodir;
AudioFileSourceSD *source = NULL;
AudioGeneratorWAV *wav = NULL;
AudioOutputI2S *output;

// ESP32-targz from https://bit.ly/2OvCUnG
#define DEST_FS_USES_SD     // The library requires this to be defined ahead of the include, it's stupid, and it only works this way, ugh!
#include <ESP32-targz.h>

String theFile = "";
String fileName = "";   // this is the file name received from the server


boolean findOneFile()
{
        Serial.println( "findOneFile" );

        HTTPClient http;

        http.begin(serverDomain, serverPort, "/onefilename");

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                //Serial.print( "Server response code: " );
                //Serial.println( httpCode );
                return false;
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        // Serial.print( "Size: " );
        // Serial.println( len );

        fileName = http.getString();
        if ( !fileName.equals( "nofiles" ) )
        {
                Serial.print( "fileName: " );
                Serial.println( fileName );
        }

        http.end();
        return true;
}

boolean getFileSaveToSDCard()
{
        HTTPClient http;

        http.begin(serverDomain, serverPort, "/download?file=" + fileName);

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                Serial.print( "Server response code: " );
                Serial.println( httpCode );
                return false;
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        Serial.print( "Size: " );
        Serial.println( len );

        // create buffer for read
        uint8_t buff[130] = { 0 };

        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        enableOneSPI( DisplaySDCS );

        File myFile = SD.open( "/" + fileName, "wb" );
        if ( myFile )
        {
                Serial.print( "SD file opened for write: " );
                Serial.println( fileName );
        }
        else
        {
                Serial.print( "Error opening new file for writing: " );
                Serial.println( fileName );
                return false;
        }

        int startTime = millis();
        int bytesReceived = 0;
        boolean buffirst = true;

        // read data from server
        while( http.connected() && (len > 0 || len == -1))
        {
          size_t size = stream->available();
          byte swap;
          if(size) 
          {
              int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size) );
              bytesReceived += c;

              // Use in case of endian problem
              /*
              byte swapper = 0;
              for ( int n = 0; n<130; n = n+2 )
              {
                swapper = buff[n];
                buff[n] = buff[n+1];
                buff[n+1] = swapper;
              }
              */
                            
              myFile.write( buff, c );
              if(len > 0) 
              {
                      len -= c;
              }
              
              if (buffirst)
              {
                Serial.println("file contents");
                Serial.println( buff[0] );
                Serial.println( buff[1] );
                Serial.println( buff[2] );
                Serial.println( buff[3] );
                buffirst = false;
              }              
          }
          smartDelay(1);
        }

        myFile.close();
        http.end();

        Serial.print( "Bytes received " );
        Serial.print( bytesReceived );
        Serial.print( " in " );
        Serial.print( ( millis() - startTime ) / 1000 );
        Serial.print( " seconds " );
        if ( ( ( millis() - startTime ) / 1000 ) > 0 )
        {
                Serial.print( bytesReceived / ( ( millis() - startTime ) / 1000 ) );
                Serial.print( " bytes/second" );
        }
        Serial.println( " " );

        return true;
}

boolean streamFileToDisplay()
{
        Serial.println( "streamFileToDisplay" );

        if ( !sdCardValid ) return false;

        Serial.print( "file: " );
        Serial.println( fileName );

        File myFile = SD.open("/" + fileName, "rb" );
        if ( myFile )
        {
                if (String(myFile.name()).endsWith( MJPEG_FILENAME ))
                {
                        streamVideo( myFile );
                }
        }
        else
        {
                Serial.print( "File did not open " );
                Serial.println( myFile.name() );
                return false;
        }

        return true;
}

boolean deleteFileFromServer()
{
        Serial.println( "deleteFileFromServer" );

        HTTPClient http;

        http.begin(serverDomain, serverPort, "/delete?file=" + fileName );

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                Serial.print( "Server response code: " );
                Serial.println( httpCode );
                return false;
        }

        http.end();
        return true;
}

boolean deleteFileFromSDCard()
{
        Serial.println( "deleteFileFromSDCard" );

        if( SD.remove( "/" + fileName ) )
        {
                Serial.println("File deleted");
        } else
        {
                Serial.println("Delete failed");
        }
}

void enableOneSPI( int deviceCS )
{
        if ( deviceCS != DisplayCS ) { disableSPIDevice( DisplayCS ); }
        if ( deviceCS != DisplaySDCS ) { disableSPIDevice( DisplaySDCS ); }
        if ( deviceCS != DisplayRST ) { disableSPIDevice( DisplayRST ); }
        if ( deviceCS != DisplayDC ) { disableSPIDevice( DisplayDC ); }
        //Serial.print( F("Enabling SPI device on GPIO ") );
        //Serial.println( deviceCS );

        pinMode(deviceCS, OUTPUT);
        digitalWrite(deviceCS, LOW);
}

void disableSPIDevice( int deviceCS )
{
        //Serial.print( F("Disabling SPI device on pin ") );
        //Serial.println( deviceCS );
        pinMode(deviceCS, OUTPUT);
        digitalWrite(deviceCS, HIGH);
}

void backlighton()
{
        for ( int i=0; i<NUM_LEDS; i++ )
        {
                leds[i] = CRGB::Black;
        }

        leds[3] = CRGB::White;
        leds[2] = CRGB::White;
        leds[1] = CRGB::White;

        leds[9] = CRGB::White;
        leds[8] = CRGB::White;
        leds[7] = CRGB::White;

        leds[15] = CRGB::White;
        leds[14] = CRGB::White;
        leds[13] = CRGB::White;

        FastLED.show();
}

void backlightoff()
{
        for ( int i=0; i<NUM_LEDS; i++ )
        {
                leds[i] = CRGB::Black;
        }
        FastLED.show();
}


void printDirectory(File dir, int numTabs) 
{
        while (true)
        {
                File entry =  dir.openNextFile();
                if (!entry) {
                        // no more files
                        break;
                }

                for (uint8_t i = 0; i < numTabs; i++) {
                        Serial.print('\t');
                }

                Serial.print(entry.name());

                if (entry.isDirectory()) {
                        Serial.println("/");
                        printDirectory(entry, numTabs + 1);
                } else {
                        // files have sizes, directories do not
                        Serial.print("\t\t");
                        Serial.println(entry.size(), DEC);
                }
                entry.close();
        }
}


/*
 * Streams an mjpeg file from an SDRAM card to the display
 */
 
void streamVideo( File vFile )
{
    Serial.print("streamVideo for ");
    Serial.println( vFile.name() );

    if (!vFile || vFile.isDirectory())
    {
      Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
      gfx->println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
    }
    else
    {
      Serial.print( "Opened " );
      Serial.println( vFile.name() );

      if ( firstTime )
      {
        mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
        mjpeg.setup(vFile /* file */, mjpeg_buf /* buffer */, gfx /* driver */, true /* multitask */, firstTime );
        firstTime = false;
      }
      else
      {
        mjpeg.setup(vFile /* file */, mjpeg_buf /* buffer */, gfx /* driver */, true /* multitask */, firstTime );
      }

      long timer = 0;
      long framecount = 0;
      long timecount = millis();

      if (!mjpeg_buf)
      {
        Serial.println(F("mjpeg_buf malloc failed!"));
      }
      else
      {
        boolean running = true;

        while ( running )
        {
          // Pace to 15 frames per second
          
          if ( millis() > timer )
          {
            running = mjpeg.readMjpegBuf();            
            framecount++;
            timer = millis() + ( 1000 / 15 );
          }

          // Stream a frame to the display
          mjpeg.drawJpg();

          if ( (wav) && ( wav->isRunning() ) ) 
          {
            if ( !wav->loop() ) wav->stop();
          }           
        }
      }
      Serial.println(F("MJPEG video end"));
      vFile.close();
    }
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

/*
 * Recursively deletes files and directory
 */

void rmdirRf(File fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = SD.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                rmdirRf(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  DELETING FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());

            if(SD.remove( file.name() )){
                Serial.println("  File deleted");
            } else {
                Serial.println("  Delete failed");
            }
        }
        file = root.openNextFile();
    }
}

/*
 * Decompress a tar file
 * Uses file name defined in theFile global
 */

int mycount = 0;

void myTarMessageCallback(const char* format, ...)
{ Serial.println( mycount++ );
  return;
}

boolean tarDecompress()
{
        
  /*
        // if exists
        String tmpme = "/TMP";
        SD.mkdir( tmpme );

        File dir = SD.open( tmpme );
        if (! dir ) 
        {
          Serial.print( "tarDecompress failed: " );
          Serial.println( tmpme );
        }
        rmdirRf(dir, tmpme.c_str(), 1);
        if(SD.rmdir( tmpme )){
            Serial.print("Dir removed ");
            Serial.println( tmpme );
        } else {
            Serial.print("Dir remove failed ");
            Serial.println( tmpme );
        }
*/
        
  bool ret = false;
  const char* tarGzFile = "/tests6.tar.gz";
  const char * mytmpdest = "/tempo"; // for md5 tests

  Serial.println( "Expanding archive" );

  TarGzUnpacker *TARGZUnpacker = new TarGzUnpacker();

  TARGZUnpacker->haltOnError( true ); // stop on fail (manual restart/reset required)
  TARGZUnpacker->setTarVerify( true ); // true = enables health checks but slows down the overall process

  TARGZUnpacker->setTarStatusProgressCallback( BaseUnpacker::defaultTarStatusProgressCallback ); // print the filenames as they're expanded
  TARGZUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
  TARGZUnpacker->setGzProgressCallback( BaseUnpacker::defaultProgressCallback ); // targzNullProgressCallback or defaultProgressCallback
  TARGZUnpacker->setLoggerCallback( BaseUnpacker::targzPrintLoggerCallback  );    // gz log verbosity
  TARGZUnpacker->setTarProgressCallback( BaseUnpacker::defaultProgressCallback ); // prints the untarring progress for each individual file
  TARGZUnpacker->setTarMessageCallback( myTarMessageCallback ); // tar log verbosity

  Serial.println("Testing tarGzExpander without intermediate file");
  if( !TARGZUnpacker->tarGzExpander(tarGzFS, tarGzFile, tarGzFS, mytmpdest, nullptr ) ) {
    Serial.print("tarGzExpander+intermediate file failed with return code ");
    Serial.println( TARGZUnpacker->tarGzGetError() );
  } else {
    ret = true;
  }

  if ( ret )
  {
    Serial.println( "Expanded successfully" );
  }
  else
  {
    Serial.println( "Expand failed" );
  }
  return ret;
        
}

void setup()
{
  Serial.begin(9600);
  smartDelay(2000);

  Serial.println("Reflections ShowRunner2");

  WiFiMulti.addAP(NETWORK, PASSWORD);
  Serial.print("Waiting for WiFi... ");    
  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());

  enableOneSPI( DisplaySDCS );

  if ( !SD.begin(DisplaySDCS, SPI, 80000000) ) /* SPI bus mode */
  {
          Serial.println("SD init failed");
          sdCardValid = false;
  }

  sdCardValid = true;

  // Configure the I2S audio player
  audioLogger = &Serial;
  source = new AudioFileSourceSD();
  output = new AudioOutputI2S();
  output->SetPinout( bclk /* bclkPin */, wclk /* wclkPin */, dout /* doutPin */);
  output->SetGain( 0.900 );
  wav = new AudioGeneratorWAV();

  enableOneSPI( DisplayCS );

  // Init Video
  gfx->begin();
  gfx->fillScreen(YELLOW);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  backlighton();

  enableOneSPI( DisplaySDCS );

  String musicname = "/Duet.wav";
  File file = SD.open( musicname, "r" );
  if (file)
  {
          source->close();
          if (source->open( musicname.c_str() ) )
          {
                  Serial.print( "Playing audio file: " );
                  Serial.println( musicname );
                  wav->begin(source, output);
          }
          else
          {
                  Serial.print( "Error playing audio file: " );
                  Serial.println( musicname );
          }
  }

  while ( ! findOneFile() ) { Serial.print("."); smartDelay(1000); }
  getFileSaveToSDCard();
  tarDecompress();
}

void loop() 
{

/*
    if ( findOneFile() )
    {
      if ( getFileSaveToSDCard() )
      {

      if ( tarDecompress() )
      {
        Serial.println( "tarDecompress() success" );
      }
      else
      {
        Serial.println( "tarDecompress() fail" );

      }

        if ( streamFileToDisplay() )
        {
          if ( deleteFileFromServer() )
          {
          }
          else
          {
            return;
          }

          if ( deleteFileFromSDCard() )
          {
          }
          else
          {
            return;
          }
        }

      }
    }
 */

 smartDelay( 5000 );
}
