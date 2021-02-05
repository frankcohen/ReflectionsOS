/*
 * Stream the start-up Reflections video to the display
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
 * January 17, 2021

 Note: To play 240x240 MJPEG uncompressed files requires the audio at
  a sample rate of 8000 kHz, 16 bits, stereo format, WAVE format
 
 Using Arduino_GFX class by @moononournation
 https://github.com/moononournation/Arduino_GFX
 For speed and support of MPEG video

 Thank you Pawel A. Hernik (https://youtu.be/o3AqITHf0mo) for tips on how to stream 
 raw/uncompressed video to the display.

 Thank you Earle F. Philhower, III, earlephilhower@yahoo.com, for the WAV audio
 player library at https://github.com/earlephilhower/ESP8266Audio
*/

#include <SD.h>
#include <SD_MMC.h>
#include <Arduino_GFX_Library.h>
#include <FastLED.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// For this sketch, you need connected SD card with '.wav' music files in the root
// directory. 

#define DATA_PIN    27
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          200

#define SPI_SPEED SD_SCK_MHZ(40)

File audiodir;
AudioFileSourceSD *source = NULL;
AudioGeneratorWAV *wav = NULL;
AudioOutputI2S *output;

#define MJPEG_FILENAME ".mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)

#define TFT_BRIGHTNESS 250  // Hearing static over the I2S speaker when brightness > 200

#define SCK 18
#define MOSI 23
#define MISO 19
#define SS 4
#define TFT_BL 33 // Just so it doesn't conflict with something else

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
File dir;

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
      
      if (!mjpeg_buf)
      {
        Serial.println(F("mjpeg_buf malloc failed!"));
      }
      else
      {
        // Stream video to display
        while (mjpeg.readMjpegBuf())
        {
          // Play video
          mjpeg.drawJpg();

          if ( (wav) && ( wav->isRunning() ) ) 
          {
            if ( !wav->loop() ) wav->stop();
          } 
          
        }
        Serial.println(F("MJPEG video end"));
        vFile.close();
      }
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

void setup()
{
  Serial.begin(115200);
  Serial.println("SD MJPEG Video");
  smartDelay(2000);

  audioLogger = &Serial;  
  source = new AudioFileSourceSD();
  output = new AudioOutputI2S();
  output -> SetPinout( 26 /* bclkPin */, 25 /* wclkPin */, 33 /* doutPin */);
  output -> SetGain( 0.900 );
  wav = new AudioGeneratorWAV();
  
  enableOneSPI( DisplayCS );

  // Init Video
  gfx->begin();
  gfx->fillScreen(YELLOW);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  for ( int i=0; i<NUM_LEDS; i++ )
  {
    leds[i] = CRGB::White;
  }
  FastLED.show();  

/*

#ifdef TFT_BL
  Serial.println( "TFT_BL setting");
  ledcAttachPin(TFT_BL, 1);     // assign TFT_BL pin to channel 1
  ledcSetup(1, 12000, 8);       // 12 kHz PWM, 8-bit resolution
  ledcWrite(1, TFT_BRIGHTNESS); // brightness 0 - 255
#endif
/*
 * 
 */
  enableOneSPI( DisplaySDCS );

  // NOTE: SD.begin(...) should be called AFTER AudioOutputSPDIF() 
  //       to takover the the SPI pins if they share some with I2S
  //       (i.e. D8 on Wemos D1 mini is both I2S BCK and SPI SS)

  if (!SD.begin(DisplaySDCS, SPI, 80000000)) /* SPI bus mode */
  {
    Serial.print( "SD card on GPIO " );
    Serial.print( DisplaySDCS );
    Serial.println( " failed to start"); 
    gfx->println(F("ERROR: SD card mount failed"));
    gfx->fillScreen(BLUE);    
    while(1);
  }
  else
  {
    sdCardValid = true;    
  }
}

void loop()
{

  if ( sdCardValid )
  {

    if ((wav) && (wav->isRunning())) {
      if (!wav->loop()) wav->stop();
    } 
    else
    {
      File audiofile = SD.open("/Duet.wav");
      if (audiofile) {
          source->close();
          if (source->open(audiofile.name())) { 
            Serial.printf_P(PSTR("Playing '%s' from SD card...\n"), audiofile.name());
            wav->begin(source, output);
          } 
          else
          {
            Serial.printf_P(PSTR("Error opening '%s'\n"), audiofile.name());
          }
        } 
      }
      
      dir = SD.open("/");
      if ( ! dir )
      {
        Serial.println( "No result for opening the directory" );
      }
      else
      {
        File file = dir.openNextFile();
                                                                    
        while ( file )
        {
          //Serial.print( "file=");
          //Serial.println( file.name() );
          
          if (file)
          {      
            if (String(file.name()).endsWith( MJPEG_FILENAME )) 
            {
              streamVideo( file );
            }
          }
          else
          {
            Serial.print( "File did not open " );
            Serial.println( file.name() );
          }
  
          file = dir.openNextFile();
        }
      }
  }
  else
  {
  }
  smartDelay(2000);
}
