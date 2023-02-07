#include <Arduino_GFX_Library.h>
#include <SPI.h>

// TFT Display
#define Display_SPI_MOSI  35
#define Display_SPI_MISO  -1
#define Display_SPI_SCK   36
#define Display_SPI_DC    5
#define Display_SPI_BK    6
#define Display_SPI_CS    12
#define Display_SPI_RST   0

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, Display_SPI_SCK, Display_SPI_MOSI, Display_SPI_MISO);
static Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 2 /* rotation */, false /* IPS */);

void setup(void) 
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.println("ThingTwo display test");

    pinMode(Display_SPI_BK, OUTPUT);
    digitalWrite(Display_SPI_BK, LOW);

    pinMode(Display_SPI_DC, OUTPUT);
    pinMode(Display_SPI_CS, OUTPUT);

    gfx->begin();
    gfx->fillScreen(BLUE);

    gfx->setCursor(10, 10);
    gfx->setTextColor(RED);
    gfx->println("Hello World!");

    delay(5000); // 5 seconds
}

void loop()
{
    gfx->setCursor(random(gfx->width()), random(gfx->height()));
    gfx->setTextColor(random(0xffff), random(0xffff));
    gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
    gfx->println("Hello World!");

    delay(1000); // 1 second
}
