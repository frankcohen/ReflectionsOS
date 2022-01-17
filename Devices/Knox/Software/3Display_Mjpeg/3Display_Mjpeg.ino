#include <SD.h>

//#define MJPEG_FILENAME "/IronFireworks2.mjpeg"
//#define MJPEG_FILENAME "/Little-Kids.mjpeg"
//#define MJPEG_FILENAME "/Stars.mjpeg"
//#define MJPEG_FILENAME "/cat5.mjpeg"
#define MJPEG_FILENAME "/Little-Kids2.mjpeg"
//#define MJPEG_FILENAME "/ReflectionsWaves2021.mjpeg"

#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame

#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(16 /* DC */, 32 /* CS */, 18 /* SCK */, 23 /* MOSI */, 19 /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, 17, 2 /* rotation */, false /* IPS */);

#include "MjpegClass.h"
static MjpegClass mjpeg;

/* variables */
static int total_frames = 0;
static unsigned long total_read_video = 0;
static unsigned long total_decode_video = 0;
static unsigned long total_show_video = 0;
static unsigned long start_ms, curr_ms;

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long start = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  total_show_video += millis() - start;
  return 1;
}

void setup()
{
  Serial.begin(115200);
  // while (!Serial);
  Serial.println("MJPEG Movie Player");
  delay(2000);

  if ( !SD.begin(4, SPI, 80000000) ) /* SPI bus mode */
  {
    Serial.println(F("SD card failed"));
  }
  else
  {
    Serial.println(F("SD card mounted"));
  }

  delay(1000);
  
  // Init Display
  gfx->begin();
  gfx->fillScreen(YELLOW);
  gfx->invertDisplay(true);
  gfx->setRotation(2);

  delay(1000);

  File mjpegFile = SD.open(MJPEG_FILENAME, FILE_READ);

  if (!mjpegFile || mjpegFile.isDirectory())
  {
    Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
  }
  else
  {
    uint8_t *mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
    if (!mjpeg_buf)
    {
      Serial.println(F("mjpeg_buf malloc failed!"));
    }
    else
    {
      Serial.println(F("MJPEG start"));

      start_ms = millis();
      curr_ms = millis();
      mjpeg.setup(
          &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
          0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);

      while (mjpegFile.available())
      {
        // Read video
        mjpeg.readMjpegBuf();
        total_read_video += millis() - curr_ms;
        curr_ms = millis();

        // Play video
        mjpeg.drawJpg();
        total_decode_video += millis() - curr_ms;

        curr_ms = millis();
        total_frames++;          
       }

       int time_used = millis() - start_ms;
       Serial.println(F("MJPEG end"));
       mjpegFile.close();
       float fps = 1000.0 * total_frames / time_used;
       total_decode_video -= total_show_video;
       Serial.printf("Total frames: %d\n", total_frames);
       Serial.printf("Time used: %d ms\n", time_used);
       Serial.printf("Average FPS: %0.1f\n", fps);
       Serial.printf("Read MJPEG: %lu ms (%0.1f %%)\n", total_read_video, 100.0 * total_read_video / time_used);
       Serial.printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video, 100.0 * total_decode_video / time_used);
       Serial.printf("Show video: %lu ms (%0.1f %%)\n", total_show_video, 100.0 * total_show_video / time_used);
      }
    }
}

void loop()
{
}
