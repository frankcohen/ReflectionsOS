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
#include <U8g2lib.h>
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

/* more fonts at: https://github.com/moononournation/ArduinoFreeFontFile.git */

void setup(void)
{
    gfx->begin();
    gfx->fillScreen(BLACK);
    gfx->setUTF8Print(true); // enable UTF8 support for the Arduino print() function

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    /* U8g2 font list: https://github.com/olikraus/u8g2/wiki/fntlistall */
    /* U8g2 Unifont list: https://github.com/olikraus/u8g2/wiki/fntgrpunifont */
    gfx->setFont(u8g2_font_unifont_tr);
    gfx->setTextColor(RED);
    gfx->setCursor(0, 16);
    gfx->println("Hello World!");

    gfx->setFont(u8g2_font_unifont_t_polish);
    gfx->setTextColor(YELLOW);
    gfx->setCursor(0, 32);
    gfx->println("Witaj świecie!");

    gfx->setFont(u8g2_font_unifont_t_vietnamese1);
    gfx->setTextColor(GREEN);
    gfx->setCursor(0, 48);
    gfx->println("Chào thế giới!");

#ifdef U8G2_USE_LARGE_FONTS
    gfx->setFont(u8g2_font_unifont_t_chinese2);
    gfx->setTextColor(CYAN);
    gfx->setCursor(0, 64);
    gfx->println("世界你好!");

    gfx->setFont(u8g2_font_unifont_t_japanese1);
    gfx->setTextColor(BLUE);
    gfx->setCursor(0, 80);
    gfx->println("こんにちは世界!");

    gfx->setFont(u8g2_font_unifont_t_korean1);
    gfx->setTextColor(MAGENTA);
    gfx->setCursor(0, 96);
    gfx->println("안녕하세요, 세계입니다!");
#endif // U8G2_USE_LARGE_FONTS
}

void loop()
{
}
