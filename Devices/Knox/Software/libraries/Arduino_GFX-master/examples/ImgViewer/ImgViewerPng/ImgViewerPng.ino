/*******************************************************************************
 * PNG Image Viewer
 * This is a simple PNG image viewer example
 * Image Source: https://github.com/logos
 *
 * Dependent libraries:
 * PNGdec: https://github.com/bitbank2/PNGdec.git
 * 
 * Setup steps:
 * 1. Change your LCD parameters in Arduino_GFX setting
 * 2. Upload PNG file
 *   FFat (ESP32):
 *     upload FFat (FatFS) data with ESP32 Sketch Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *   LittleFS (ESP32 / ESP8266 / Pico):
 *     upload LittleFS data with ESP8266 LittleFS Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *     ESP8266: https://github.com/earlephilhower/arduino-esp8266littlefs-plugin
 *     Pico: https://github.com/earlephilhower/arduino-pico-littlefs-plugin.git
 *   SPIFFS (ESP32):
 *     upload SPIFFS data with ESP32 Sketch Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *   SD:
 *     Most Arduino system built-in support SD file system.
 *     Wio Terminal require extra dependant Libraries:
 *     - Seeed_Arduino_FS: https://github.com/Seeed-Studio/Seeed_Arduino_FS.git
 *     - Seeed_Arduino_SFUD: https://github.com/Seeed-Studio/Seeed_Arduino_SFUD.git
 ******************************************************************************/
#define PNG_FILENAME "/octocat.png"
#define PNG_4BPP_FILENAME "/octocat-4bpp.png"

/*******************************************************************************
 * Start of Arduino_GFX setting
 * 
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: TFT_CS:  9, TFT_DC:  8, TFT_RST:  7, TFT_BL:  6
 * ESP32 various dev board     : TFT_CS:  5, TFT_DC: 27, TFT_RST: 33, TFT_BL: 22
 * ESP8266 various dev board   : TFT_CS: 15, TFT_DC:  4, TFT_RST:  2, TFT_BL:  5
 * Raspberry Pi Pico dev board : TFT_CS: 17, TFT_DC: 27, TFT_RST: 26, TFT_BL: 28
 * RTL872x various dev board   : TFT_CS: 18, TFT_DC: 17, TFT_RST:  2, TFT_BL: 23
 * Seeeduino XIAO dev board    : TFT_CS:  3, TFT_DC:  2, TFT_RST:  1, TFT_BL:  0
 * Teensy 4.1 dev board        : TFT_CS: 39, TFT_DC: 41, TFT_RST: 40, TFT_BL: 22
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 3 /* rotation */, false /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#include <Seeed_FS.h>
#include <SD/Seeed_SD.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
#include <LittleFS.h>
#include <SD.h>
#elif defined(ESP32)
#include <FFat.h>
// #include <LittleFS.h>
#include <SPIFFS.h>
#include <SD.h>
#elif defined(ESP8266)
#include <LittleFS.h>
#include <SD.h>
#else
#include <SD.h>
#endif

#include <PNGdec.h>
PNG png;

int16_t w, h, xOffset, yOffset;

// Functions to access a file on the SD card
File pngFile;

void *myOpen(const char *filename, int32_t *size)
{
/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
  pngFile = SD.open(filename, "r");
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  pngFile = LittleFS.open(filename, "r");
  // pngFile = SD.open(filename, "r");
#elif defined(ESP32)
  pngFile = FFat.open(filename, "r");
  // pngFile = LittleFS.open(filename, "r");
  // pngFile = SPIFFS.open(filename, "r");
  // pngFile = SD.open(filename, "r");
#elif defined(ESP8266)
  pngFile = LittleFS.open(filename, "r");
  // pngFile = SD.open(filename, "r");
#else
  pngFile = SD.open(filename, FILE_READ);
#endif

  if (!pngFile || pngFile.isDirectory())
  {
    Serial.println(F("ERROR: Failed to open " PNG_FILENAME " file for reading"));
    gfx->println(F("ERROR: Failed to open " PNG_FILENAME " file for reading"));
  }
  else
  {
    *size = pngFile.size();
  }

  return &pngFile;
}

void myClose(void *handle)
{
  if (pngFile)
    pngFile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!pngFile)
    return 0;
  return pngFile.read(buffer, length);
}
int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!pngFile)
    return 0;
  return pngFile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{
  uint16_t usPixels[320];
  uint8_t usMask[320];

  // Serial.printf("Draw pos = 0,%d. size = %d x 1\n", pDraw->y, pDraw->iWidth);
  png.getLineAsRGB565(pDraw, usPixels, RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);
  gfx->draw16bitRGBBitmap(xOffset, yOffset + pDraw->y, usPixels, usMask, pDraw->iWidth, 1);
}

void setup()
{
  Serial.begin(115200);
  // while (!Serial);
  Serial.println("PNG Image Viewer");

  // Init Display
  gfx->begin();
  gfx->fillScreen(BLACK);

  w = gfx->width(), h = gfx->height();
  gfx->fillScreen(BLACK);
  for (int16_t x = 0; x < w; x += 5)
  {
    gfx->drawFastVLine(x, 0, h, PINK);
  }

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  if (!LittleFS.begin())
  // if (!SD.begin(SS))
#elif defined(ESP32)
  if (!FFat.begin())
  // if (!LittleFS.begin())
  // if (!SPIFFS.begin())
  // if (!SD.begin(SS))
#elif defined(ESP8266)
  if (!LittleFS.begin())
  // if (!SD.begin(SS))
#else
  if (!SD.begin())
#endif
  {
    Serial.println(F("ERROR: File System Mount Failed!"));
    gfx->println(F("ERROR: File System Mount Failed!"));
  }
  else
  {
    unsigned long start = millis();
    int rc;
    rc = png.open(PNG_FILENAME, myOpen, myClose, myRead, mySeek, PNGDraw);
    if (rc == PNG_SUCCESS)
    {
      xOffset = (w - png.getWidth()) / 2;
      yOffset = (h - png.getHeight()) / 2;

      rc = png.decode(NULL, 0);

      Serial.printf("Time used: %lu\n", xOffset, yOffset, millis() - start);
      Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
      png.close();
    }
  }

  delay(5000); // 5 seconds
}

void loop()
{
  unsigned long start = millis();
  int rc;
  rc = png.open(PNG_4BPP_FILENAME, myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rc == PNG_SUCCESS)
  {
    // random draw position
    int16_t pw = png.getWidth();
    int16_t ph = png.getHeight();
    xOffset = random(w) - (pw / 2);
    yOffset = random(h) - (ph / 2);

    rc = png.decode(NULL, 0);

    Serial.printf("Draw offset: (%d, %d), time used: %lu\n", xOffset, yOffset, millis() - start);
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    png.close();
  }

  delay(1000); // 1 second
}
