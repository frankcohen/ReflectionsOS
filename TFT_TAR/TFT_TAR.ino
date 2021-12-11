#define DEST_FS_USES_SD
#include <ESP32-targz.h>
#include <SD.h>
#include <Arduino_GFX_Library.h>
#include "tar.h"

#define MJPEG_FILENAME ".mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)

bool test_succeeded = false;

bool test_tarExpander() {
  bool ret = false;
  const char* tarFile = "/DemoReel3.tar";
  myPackage.folder = "/"; // for md5 tests

  Serial.printf("Testing tarExpander\n");

  TarUnpacker *TARUnpacker = new TarUnpacker();

  TARUnpacker->haltOnError( true ); // stop on fail (manual restart/reset required)
  TARUnpacker->setTarVerify( true ); // true = enables health checks but slows down the overall process

  TARUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
  TARUnpacker->setLoggerCallback( BaseUnpacker::targzPrintLoggerCallback  );  // log verbosity
  TARUnpacker->setTarProgressCallback( BaseUnpacker::defaultProgressCallback ); // prints the untarring progress for each individual file
  TARUnpacker->setTarStatusProgressCallback( BaseUnpacker::defaultTarStatusProgressCallback ); // print the filenames as they're expanded
  TARUnpacker->setTarMessageCallback( BaseUnpacker::targzPrintLoggerCallback ); // tar log verbosity

  if(  !TARUnpacker->tarExpander(tarGzFS, tarFile, tarGzFS, myPackage.folder ) ) {
    Serial.printf("tarExpander failed with return code #%d\n", TARUnpacker->tarGzGetError() );
  } else {
    ret = true;
  }
  return ret;
}

#define TFT_BRIGHTNESS 250  // Hearing static over the I2S speaker when brightness > 200

#define SCK     18
#define MOSI    23
#define MISO    19
#define SS      5

#define DisplayCS   5   //TFT display on Adafruit's ST7789 card
#define DisplaySDCS 4    //SD card on the Adafruit ST7789 card
#define DisplayRST  17    //Reset for Adafruit's ST7789 card
#define DisplayDC   16   //DC for Adafruit's ST7789 card

// ST7789 Display
Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);


#include "MjpegClass.h"
static MjpegClass mjpeg;
boolean sdCardValid = false;
boolean firstTime = true;
uint8_t *mjpeg_buf;
File dir;

void enableOneSPI( int deviceCS ) {
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

void setup() {
  Serial.begin(115200);
  smartDelay(1000);

  if (!SD.begin(DisplaySDCS)) {
    Serial.print( "SD card on GPIO " );
    Serial.print( DisplaySDCS );
    Serial.println( " failed to start"); 
    gfx->println(F("ERROR: SD card mount failed"));
    gfx->fillScreen(BLUE);    
    while(1);
  }
  else {
    sdCardValid = true;    
  }

  Serial.println("EXTRACTING FILE");

  if (!tarGzFS.begin()) {
    Serial.printf("%s Mount Failed, halting\n", FS_NAME );
    while(1) yield();
  } else {
    Serial.printf("%s Mount Successful\n", FS_NAME);
  }

  test_succeeded = test_tarExpander();

  if( test_succeeded ) {
    Serial.printf("FILES EXTRACTED\n");
  }

  Serial.println("SD MJPEG Video");
  //smartDelay(2000);

   enableOneSPI( DisplayCS );

  // Init Video
  gfx->begin();
  gfx->fillScreen(YELLOW);

}

void loop() {
  if ( sdCardValid ) {  

      dir = SD.open("/DemoReel3");
      if ( ! dir ) {
        Serial.println( "No result for opening the directory" );
      }

      else {
        File file = dir.openNextFile();
                                                                    
        while ( file )
        {
          Serial.print( "file=");
          Serial.println( file.name() );
          
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
  else {
  }
  smartDelay(2000);
}
