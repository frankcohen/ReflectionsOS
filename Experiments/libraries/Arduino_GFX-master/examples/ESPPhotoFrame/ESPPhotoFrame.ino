/*******************************************************************************
 * ESP32 Photo Frame
 * This is a simple IoT photo frame sample
 * Please find more details at instructables:
 * https://www.instructables.com/id/Face-Aware-OSD-Photo-Frame/
 * 
 * Setup steps:
 * 1. Fill your own SSID_NAME, SSID_PASSWORD and URL_TEMPLATE
 * 2. Change your LCD parameters in Arduino_GFX setting
 ******************************************************************************/

/* WiFi settings */
#define SSID_NAME "YourAP"
#define SSID_PASSWORD "PleaseInputYourPasswordHere"

#define URL_TEMPLATE "http://YourServerNameOrIP:8080/?w=%d&h=%d"

#define HTTP_TIMEOUT 60000 // in ms, wait a while for server processing

/*******************************************************************************
 * Start of Arduino_GFX setting
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

/* first check if selected specific hardware */
// #define ESP32_LCDKIT_SPI
// #define WT32_SC01
#if defined(ESP32_LCDKIT_SPI)
#define TFT_BL 23
Arduino_DataBus *bus = new Arduino_ESP32SPI(19 /* DC */, 5 /* CS */, 22 /* SCK */, 21 /* MOSI */, 27 /* MISO */);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, 18 /* RST */, 1 /* rotation */);

#elif defined(WT32_SC01)
#define TFT_BL 23
Arduino_DataBus *bus = new Arduino_ESP32SPI(21 /* DC */, 15 /* CS */, 14 /* SCK */, 13 /* MOSI */, -1 /* MISO */);
Arduino_ST7796 *gfx = new Arduino_ST7796(bus, 22 /* RST */, 3 /* rotation */);

/* Wio Terminal */
#elif defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#define TFT_BL LCD_BACKLIGHT
Arduino_HWSPI *bus = new Arduino_HWSPI(LCD_DC /* DC */, LCD_SS_PIN /* CS */);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, -1 /* RST */, 3 /* rotation */);

/* M5Stack */
#elif defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#define TFT_BL 32
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(27 /* DC */, 14 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341_M5STACK *gfx = new Arduino_ILI9341_M5STACK(bus, 33 /* RST */, 1 /* rotation */);

/* Odroid-Go */
#elif defined(ARDUINO_ODROID_ESP32)
#define TFT_BL 14
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(21 /* DC */, 5 /* CS */, SCK, MOSI, MISO);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, -1 /* RST */, 3 /* rotation */);
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus,  -1 /* RST */, 1 /* rotation */, true /* IPS */);

/* TTGO T-Watch */
#elif defined(ARDUINO_T) || defined(ARDUINO_TWATCH_BASE) || defined(ARDUINO_TWATCH_2020_V1) || defined(ARDUINO_TWATCH_2020_V2)
#define TFT_BL 12
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(27 /* DC */, 5 /* CS */, 18 /* SCK */, 19 /* MOSI */, -1 /* MISO */);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240, 240, 0, 80);

#else /* not selected specific hardware */

#if defined(ESP32)
#define TFT_CS 5
// #define TFT_CS -1 // for display without CS pin
// #define TFT_DC 16
#define TFT_DC 27
// #define TFT_DC -1 // for display without DC pin (9-bit SPI)
// #define TFT_RST 17
#define TFT_RST 33
#define TFT_BL 22
#elif defined(ESP8266)
#define TFT_CS 15
#define TFT_DC 5
#define TFT_RST -1
// #define TFT_BL 4
#else
#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BL 6
#endif

/*
 * Step 1: Initize one databus for your display
*/

// General software SPI
// Arduino_DataBus *bus = new Arduino_SWSPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */);

// General hardware SPI
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);

// ESP32 hardware SPI, more customizable parameters
// Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);

// ESP32 parallel 8-bit
// Arduino_DataBus *bus = new Arduino_ESP32PAR8(TFT_DC, TFT_CS, 26 /* WR */, -1 /* RD */, 16 /* D0 */, 17 /* D1 */, 18 /* D2 */, 19 /* D3 */, 21 /* D4 */, 12 /* D5 */, 23 /* D6 */, 25 /* D7 */);

// ESP32 parallel 16-bit
// Almost all GPIO 0-31 used up for 16-bit and WR, disable PSRAM to gain 16 and 17 but still no GPIOs remain for CS and RD.
// CS connect to GND (enable); RD connect to Vcc (disable).
// Arduino_DataBus *bus = new Arduino_ESP32PAR16(
//     32 /* DC */, -1 /* CS */, 21 /* WR */, -1 /* RD */,
//     19 /* D0 */, 23 /* D1 */, 18 /* D2 */, 5 /* D3 */, 17 /* D4 */, 16 /* D5 */, 25 /* D6 */, 26 /* D7 */,
//     27 /* D8 */, 14 /* D9 */, 12 /* D10 */, 13 /* D11 */, 15 /* D12 */, 2 /* D13 */, 0 /* D14 */, 4 /* D15 */);

/*
 * Step 2: Initize one driver for your display
*/

// Canvas (framebuffer)
// Arduino_ST7789 *output_display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
// 16-bit color Canvas (240x320 resolution only works for ESP32 with PSRAM)
// Arduino_Canvas *gfx = new Arduino_Canvas(240 /* width */, 320 /* height */, output_display);
// Indexed color Canvas, mask_level: 0-2, larger mask level mean less color variation but can have faster index mapping
// Arduino_Canvas_Indexed *gfx = new Arduino_Canvas_Indexed(240 /* width */, 320 /* height */, output_display, 0 /* output_x */, 0 /* output_y */, MAXMASKLEVEL /* mask_level */);

// GC9A01 IPS LCD 240x240
// Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8347C IPS LCD 240x320
// Arduino_HX8347C *gfx = new Arduino_HX8347C(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8347D IPS LCD 240x320
// Arduino_HX8347D *gfx = new Arduino_HX8347D(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8352C IPS LCD 240x400
// Arduino_HX8352C *gfx = new Arduino_HX8352C(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// HX8357B IPS LCD 320x480
// Arduino_HX8357B *gfx = new Arduino_HX8357B(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// ILI9225 LCD 176x220
// Arduino_ILI9225 *gfx = new Arduino_ILI9225(bus, TFT_RST);

// ILI9341 LCD 240x320
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST, 0 /* rotation */);

// ILI9481 LCD 320x480
// Arduino_ILI9481_18bit *gfx = new Arduino_ILI9481_18bit(bus, TFT_RST, 0 /* rotation */);

// ILI9486 LCD 320x480
// Arduino_ILI9486_18bit *gfx = new Arduino_ILI9486_18bit(bus, TFT_RST, 0 /* rotation */);

// ILI9488 LCD 320x480
// Arduino_ILI9488_18bit *gfx = new Arduino_ILI9488_18bit(bus, TFT_RST, 0 /* rotation */);

// JBT6K71 LCD 240x320
// Arduino_JBT6K71 *gfx = new Arduino_JBT6K71(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 240, 320, 0, 0, 16, 0);

// R61529 IPS LCD 320x480
// Arduino_R61529 *gfx = new Arduino_R61529(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

// SEPS525 OLED 160x128
// Arduino_SEPS525 *gfx = new Arduino_SEPS525(bus, TFT_RST, 0 /* rotation */);

// SSD1283A OLED 130x130
// Arduino_SSD1283A *gfx = new Arduino_SSD1283A(bus, TFT_RST, 0 /* rotation */);

// SSD1331 OLED 96x64
// Arduino_SSD1331 *gfx = new Arduino_SSD1331(bus, TFT_RST, 0 /* rotation */);

// SSD1351 OLED 128x128
// Arduino_SSD1351 *gfx = new Arduino_SSD1351(bus, TFT_RST, 0 /* rotation */);

// ST7735 LCD
// 1.8" REDTAB 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */);
// 1.8" BLACKTAB 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 1 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */, false /* BGR */);
// 1.8" GREENTAB A 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 1 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.8" GREENTAB B 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 2 /* col offset 1 */, 3 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.8" Wide angle LCD 128x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 160 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */, false /* BGR */);
// 1.5" GREENTAB B 128x128
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 128 /* height */, 2 /* col offset 1 */, 3 /* row offset 1 */, 2 /* col offset 2 */, 1 /* row offset 2 */);
// 1.5" GREENTAB C 128x128
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 0 /* rotation */, false /* IPS */, 128 /* width */, 128 /* height */, 0 /* col offset 1 */, 32 /* row offset 1 */);
// 0.96" IPS LCD 80x160
// Arduino_ST7735 *gfx = new Arduino_ST7735(bus, TFT_RST, 3 /* rotation */, true /* IPS */, 80 /* width */, 160 /* height */, 26 /* col offset 1 */, 1 /* row offset 1 */, 26 /* col offset 2 */, 1 /* row offset 2 */);

// ST7789 LCD
// 2.4" LCD 240x320
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */);
// 2.4" IPS LCD 240x320
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
// 1.3"/1.5" square IPS LCD 240x240
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
// 1.14" IPS LCD 135x240 TTGO T-Display
// Arduino_ST7789 *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 135 /* width */, 240 /* height */, 53 /* col offset 1 */, 40 /* row offset 1 */, 52 /* col offset 2 */, 40 /* row offset 2 */);

// ST7796 LCD
// 4" LCD 320x480
// Arduino_ST7796 *gfx = new Arduino_ST7796(bus, TFT_RST, 0 /* rotation */);
// 4" IPS LCD 320x480
// Arduino_ST7796 *gfx = new Arduino_ST7796(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

#endif /* not selected specific hardware */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

#if defined(ESP32)
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <HTTPClient.h>
#else // ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include "JpegDec.h"
static JpegDec jpegDec;

static uint32_t len, offset;
static unsigned long next_show_millis = 0;

HTTPClient http;
char url[1024];

void setup()
{
  Serial.begin(115200);
  gfx->begin();
  gfx->fillScreen(BLACK);

  // setup url query value with LCD dimension
  sprintf(url, URL_TEMPLATE, gfx->width(), gfx->height());

  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

#if defined(ESP32)
  // set WDT timeout a little bit longer than HTTP timeout
  esp_task_wdt_init((HTTP_TIMEOUT / 1000) + 1, true);
  enableLoopWDT();
#endif
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // wait for WiFi connection
    delay(500);
  }
  else if (millis() < next_show_millis)
  {
    delay(1000);
  }
  else
  {
    next_show_millis = ((millis() / 60000L) + 1) * 60000L; // next minute
    Serial.print("[HTTP] begin...\n");
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();

    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    // HTTP header has been send and Server response header has been handled
    if (httpCode <= 0)
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    else
    {
      if (httpCode != HTTP_CODE_OK)
      {
        Serial.printf("[HTTP] Not OK!\n");
        delay(5000);
      }
      else
      {
        // get lenght of document (is -1 when Server sends no Content-Length header)
        len = http.getSize();
        Serial.printf("[HTTP] size: %d\n", len);

        if (len <= 0)
        {
          Serial.printf("[HTTP] Unknow content size: %d\n", len);
        }
        else
        {
          // get tcp stream
          WiFiClient *http_stream = http.getStreamPtr();
          jpegDec.prepare(http_stream_reader, http_stream);
          jpegDec.decode(JPG_SCALE_NONE, jpegDec.gfx_writer, gfx);
        }
      }
    }
    http.end();
  }

#if defined(ESP32)
  // notify WDT still working
  feedLoopWDT();
#else // ESP8266
  yield();
#endif
}

static size_t http_stream_reader(JpegDec *jpegDec, size_t index, uint8_t *buf, size_t len)
{
  WiFiClient *http_stream = (WiFiClient *)jpegDec->input;
  uint8_t wait = 0;
  if (buf)
  {
    // Serial.printf("[HTTP] read: %d\n", len);
    size_t a = http_stream->available();
    size_t r = 0;
    while ((r < len) && (wait < 10))
    {
      if (a)
      {
        r += http_stream->readBytes(buf + r, min((len - r), a));
      }
      else
      {
        delay(10);
        wait++;
      }
      a = http_stream->available();
    }
    return r;
  }
  else
  {
    // Serial.printf("[HTTP] skip: %d\n", len);
    size_t l = len;
    while ((--l) && (wait < 10))
    {
      if (http_stream->available())
      {
        http_stream->read();
      }
      else
      {
        delay(10);
        wait++;
      }
    }
    return len;
  }
}
