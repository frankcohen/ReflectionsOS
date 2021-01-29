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

 Using Arduino_GFX class by @moononournation
 https://github.com/moononournation/Arduino_GFX
 For speed and support of MPEG video

 Thank you Pawel A. Hernik (https://youtu.be/o3AqITHf0mo) for tips on how to stream 
 raw/uncompressed video to the display.
*/

#define MJPEG_FILENAME ".mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)

#include <SD.h>
#include <SD_MMC.h>
#include <Arduino_GFX_Library.h>

#define TFT_BRIGHTNESS 200  // Hearing static over the I2S speaker when brightness > 200

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
    //Serial.print( F("\nDisabling SPI device on pin ") );
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
  Serial.println("SD MJEPG Video");

  enableOneSPI( DisplayCS );

  // Init Video
  gfx->begin();
  gfx->fillScreen(YELLOW);

#ifdef TFT_BL
Serial.println( "TFT_BL setting");

  ledcAttachPin(TFT_BL, 1);     // assign TFT_BL pin to channel 1
  ledcSetup(1, 12000, 8);       // 12 kHz PWM, 8-bit resolution
  ledcWrite(1, TFT_BRIGHTNESS); // brightness 0 - 255
#endif

  enableOneSPI( DisplaySDCS );

  // Init SD card
  if (!SD.begin(DisplaySDCS, SPI, 80000000)) /* SPI bus mode */
  // if ((!SD_MMC.begin()) && (!SD_MMC.begin())) /* 4-bit SD bus mode */
  // if ((!SD_MMC.begin("/sdcard", true)) && (!SD_MMC.begin("/sdcard", true))) /* 1-bit SD bus mode */
  {
    Serial.print( "SD card on GPIO " );
    Serial.print( DisplaySDCS );
    Serial.println( " failed to start"); 
    gfx->println(F("ERROR: SD card mount failed"));
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
  smartDelay(2000);
}
