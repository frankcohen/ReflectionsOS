/*******************************************************************************
 * PROGMEM Image Viewer
 * This is a simple PROGMEM bitmap image viewer example
 * Image Source: https://support.arduino.cc/hc/en-us/articles/360020652100
 *
 * How to create custom image:
 * 1. Open image ith GIMP
 * 2. Export Image as C-Source
 * 3. Uncheck all option and check "Save as RGB565 (16-bit)"
 * 4. Revise exported file just like "Arduino_UNO_Rev3_Ok.c"
 ******************************************************************************/
#include "Arduino_UNO_Rev3_Ok.c"
#define IMG_WIDTH 100
#define IMG_HEIGHT 71

/*******************************************************************************
 * Start of Arduino_GFX setting
 * 
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: CS:  9, DC:  8, RST:  7, BL:  6
 * ESP32 various dev board     : CS:  5, DC: 27, RST: 33, BL: 22
 * ESP32-C3 various dev board  : CS:  7, DC:  2, RST:  1, BL:  3
 * ESP32-S2 various dev board  : CS: 34, DC: 35, RST: 33, BL: 21
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48
 * ESP8266 various dev board   : CS: 15, DC:  4, RST:  2, BL:  5
 * Raspberry Pi Pico dev board : CS: 17, DC: 27, RST: 26, BL: 28
 * RTL8720 BW16 old patch core : CS: 18, DC: 17, RST:  2, BL: 23
 * RTL8720_BW16 Official core  : CS:  9, DC:  8, RST:  6, BL:  3
 * RTL8722 dev board           : CS: 18, DC: 17, RST: 22, BL: 23
 * RTL8722_mini dev board      : CS: 12, DC: 14, RST: 15, BL: 13
 * Seeeduino XIAO dev board    : CS:  3, DC:  2, RST:  1, BL:  0
 * Teensy 4.1 dev board        : CS: 39, DC: 41, RST: 40, BL: 22
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

void setup()
{
  Serial.begin(115200);
  // while (!Serial);
  Serial.println("BMP Image Viewer");

  // Init Display
  gfx->begin();
  gfx->fillScreen(BLACK);

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

  gfx->draw16bitRGBBitmap(0, 0, (const uint16_t*)Arduino_UNO_Rev3_Ok, IMG_WIDTH, IMG_HEIGHT);

  delay(5000); // 5 seconds
}

void loop()
{
  int16_t x = random(gfx->width());
  int16_t y = random(gfx->height());
  gfx->draw16bitRGBBitmap(x, y, (const uint16_t*)Arduino_UNO_Rev3_Ok, IMG_WIDTH, IMG_HEIGHT);

  delay(1000); // 1 second
}
