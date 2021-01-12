#define MJPEG_FILENAME "/220_30fps.mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)
#include <SD.h>
#include <SD_MMC.h>

#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789, https://github.com/cbm80amiga/Arduino_ST7735_STM
#include <Adafruit_GFX.h>

#include <Arduino_GFX_Library.h>
#define TFT_BRIGHTNESS 128

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#define TFT_BL 32
#define SS 4
Arduino_HWSPI *bus = new Arduino_HWSPI(27 /* DC */, 14 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341_M5STACK *gfx = new Arduino_ILI9341_M5STACK(bus, 33 /* RST */, 1 /* rotation */);
#elif defined(ARDUINO_ODROID_ESP32)
#define TFT_BL 14
Arduino_HWSPI *bus = new Arduino_HWSPI(21 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
// Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, -1 /* RST */, 3 /* rotation */);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 1 /* rotation */, true /* IPS */);
#elif defined(ARDUINO_T) // TTGO T-Watch
#define TFT_BL 12
Arduino_HWSPI *bus = new Arduino_HWSPI(27 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240, 240, 0, 80);
#else /* not a specific hardware */

#define SCK 18
#define MOSI 23
#define MISO 19
#define SS 4
#define TFT_BL 33 // Just so it doesn't conflict with something else

#define DisplayCS 32     //TFT display on Adafruit's ST7789 card
#define DisplaySDCS 4    //SD card on the Adafruit ST7789 card
#define DisplayRST 17    //Reset for Adafruit's ST7789 card
#define DisplayDC 16     //DC for Adafruit's ST7789 card

/*
ESP32
GPIO 2  -> SD D0/MISO
GPIO 14 -> SD CLK
GPIO 15 -> SD CMD/MOSI
GPIO 18 -> LCD SCK
GPIO 19 -> LCD MISO
GPIO 22 -> LCD LED
GPIO 23 -> LCD MOSI
GPIO 27 -> LCD DC/RS
GPIO 33 -> LCD RST
*/
// ST7789 Display
Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);

// ILI9225 Display
// Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
// Arduino_ILI9225 *gfx = new Arduino_ILI9225(bus, DisplayRST /* RST */, 1 /* rotation */);

#endif /* not a specific hardware */

#include "MjpegClass.h"
static MjpegClass mjpeg;

void enableOneSPI( int deviceCS )
{
  if ( deviceCS != DisplayCS ) { disableSPIDevice( DisplayCS ); }
  if ( deviceCS != DisplaySDCS ) { disableSPIDevice( DisplaySDCS ); }
  if ( deviceCS != DisplayRST ) { disableSPIDevice( DisplayRST ); }
  if ( deviceCS != DisplayDC ) { disableSPIDevice( DisplayDC ); }
  Serial.print( F("\nEnabling SPI device on pin ") );
  Serial.println( deviceCS );

  pinMode(deviceCS, OUTPUT);
  digitalWrite(deviceCS, LOW);
}

void disableSPIDevice( int deviceCS )
{
    Serial.print( F("\nDisabling SPI device on pin ") );
    Serial.println( deviceCS );
    pinMode(deviceCS, OUTPUT);
    digitalWrite(deviceCS, HIGH);
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
    File vFile = SD.open(MJPEG_FILENAME);
    // File vFile = SD_MMC.open(MJPEG_FILENAME);
    if (!vFile || vFile.isDirectory())
    {
      Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
      gfx->println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
    }
    else
    {
      Serial.print( "Opened " );
      Serial.println( MJPEG_FILENAME );
      
      uint8_t *mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
      if (!mjpeg_buf)
      {
        Serial.println(F("mjpeg_buf malloc failed!"));
      }
      else
      {
        Serial.println(F("MJPEG video start"));
        mjpeg.setup(vFile, mjpeg_buf, gfx, true);
        // Read video
        
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
#ifdef TFT_BL
  delay(60000);
  ledcDetachPin(TFT_BL);
#endif
  gfx->displayOff();
  esp_deep_sleep_start();
}

void loop()
{
}
