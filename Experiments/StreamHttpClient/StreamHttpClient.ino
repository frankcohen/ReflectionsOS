/*
 * Get video files from a server and stream them to the display
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

*/

#include <WiFi.h>
#include <WiFiMulti.h>

#include "HTTPClient.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <Arduino_GFX_Library.h>
#include <FastLED.h>

#define serverDomain "54.190.177.23"
#define serverPort 80

#define DATA_PIN    27
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];
#define BRIGHTNESS  250   // Backlight brightness using WS2812 Neopixel LED units

#define SPI_SPEED SD_SCK_MHZ(40)

#define MJPEG_FILENAME ".mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)

#define SCK 18
#define MOSI 23
#define MISO 19
#define SS 4

#define DisplayCS 32     //TFT display on Adafruit's ST7789 card
#define DisplaySDCS 4    //SD card on the Adafruit ST7789 card
#define DisplayRST 17    //Reset for Adafruit's ST7789 card
#define DisplayDC 16     //DC for Adafruit's ST7789 card

// ST7789 Display
Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);

#include "MjpegClass.h"
static MjpegClass mjpeg;
boolean sdCardValid = false;
boolean firstTime = true;
uint8_t *mjpeg_buf;

WiFiMulti wifiMulti;
String fileName = "";
File dir;


/*
 * Gets one file available for download, stores the file name in fileName
 */

boolean findOneFile()
{
  Serial.println( "findONeFile" );
  // wait for WiFi connection
  if((wifiMulti.run() != WL_CONNECTED))
  {
    Serial.println( "Wifi not connected" );
    return false;
  }
      
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
  if ( ! fileName.equals( "nofiles" ) )
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
    //Serial.print( "Server response code: " );
    //Serial.println( httpCode );
    return false;
  }

  // get length of document (is -1 when Server sends no Content-Length header)
  int len = http.getSize();
  Serial.print( "Size: " );
  Serial.println( len );

  // create buffer for read
  uint8_t buff[128] = { 0 };

  // get tcp stream
  WiFiClient * stream = http.getStreamPtr();

  enableOneSPI( DisplaySDCS );
  
  File myFile = SD.open("/" + fileName, FILE_WRITE);
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
  
  // read all data from server
  while( http.connected() && (len > 0 || len == -1)) 
  {
      size_t size = stream->available();

      if(size) {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          bytesReceived += c;          
          myFile.write( buff, c );
          
          if(len > 0) {
              len -= c;
          }
      }
      smartDelay(1);
  }

  myFile.close();
  http.end();

  // Fix bad endianness from your node.js script
  Serial.println( "Fixing bad endianness from node" );
  
  byte mybuf[3];
  byte swap;
  File file1 = SD.open("/" + fileName );
  File file2 = SD.open("/" + fileName + "2", FILE_WRITE);
  int flen = file1.size();
  while ( flen )
  {
    file1.read( mybuf, 2 );
    swap = mybuf[0];
    mybuf[0] = mybuf[1];
    mybuf[1] = swap;
    file2.write( mybuf, 2 );
    flen = flen - 2;    
  }
  file1.close();
  file2.close();
  SD.remove( "/" + fileName );
  SD.rename("/" + fileName + "2", "/" + fileName );

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

/*
 * Streams a mjpeg file from an SDRAM card to the display
 */
 
void streamVideo( File vFile )
{
    if (!vFile || vFile.isDirectory())
    {
      Serial.println(F("Failed to open " MJPEG_FILENAME " file for reading"));
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

      long mlen = vFile.size();
      
      if (!mjpeg_buf)
      {
        Serial.println(F("mjpeg_buf malloc failed!"));
      }
      else
      {
        // Stream video to display
        while ( mjpeg.readMjpegBuf() )
        {
          // Play video
          mjpeg.drawJpg();
        }
        Serial.println(F("MJPEG video end"));
        vFile.close();
      }
    }
}

boolean streamFileToDisplay()
{
  Serial.println( "streamFileToDisplay" );

  if ( ! sdCardValid ) return false;

  Serial.print( "file: " );
  Serial.println( fileName );

  File myFile = SD.open("/" + fileName );
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
  // wait for WiFi connection
  if((wifiMulti.run() != WL_CONNECTED))
  {
    Serial.println( "Wifi not connected" );
    return false;
  }
      
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

void setup() {
  Serial.begin(9600);
  smartDelay(2000);
  Serial.println( "StreamHttpClient");
  Serial.println( "Connecting to Wifi");

  wifiMulti.addAP("Franks_Free_Internet_Guest", "");

  enableOneSPI( DisplaySDCS );

  Serial.begin(9600);
  if(!SD.begin( 4, SPI, 80000000  )){
    Serial.println("Card Mount Failed");
    sdCardValid = false;    
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.print( "SD card on GPIO " );
      Serial.print( DisplaySDCS );
      Serial.println( " failed to start");
      return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);    

  /*
  Serial.println("dir");
  File root = SD.open("/");
  printDirectory(root, 0);
  */
  
  sdCardValid = true;    

  enableOneSPI( DisplayCS );

  // Init Video
  gfx->begin();
  gfx->fillScreen(YELLOW);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  for ( int i=0; i<NUM_LEDS; i++ )
  {
    leds[i] = CRGB::Black;
  }

  leds[9] = CRGB::White;
  leds[8] = CRGB::White;
  leds[15] = CRGB::White;
  leds[15] = CRGB::White;
  
  FastLED.show();  

  enableOneSPI( DisplaySDCS );  
}

void printDirectory(File dir, int numTabs) {
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

  
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

String theFile = "";

void loop() {

    if ( findOneFile() )
    {
      if ( getFileSaveToSDCard() )
      {
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

    smartDelay( 5000 );
}
