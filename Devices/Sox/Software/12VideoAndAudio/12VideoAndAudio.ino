#include "Arduino.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

// Digital I/O used
#define SD_CS          4
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      27
#define I2S_BCLK      26
#define I2S_LRC       25
#define SPI_DisplayDC 16
#define SPI_DisplayCS 32
#define SPI_DisplayRST 17  

File audioDir;
File videoDir;

#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10) // memory for a single JPEG frame

#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32SPI(SPI_DisplayDC /* DC */, SPI_DisplayCS /* CS */, SPI_SCK /* SCK */, SPI_MOSI /* MOSI */, SPI_MISO /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, SPI_DisplayRST, 2 /* rotation */, false /* IPS */);

#include "MjpegClass.h"
static MjpegClass mjpeg;
uint8_t *mjpeg_buf;
File mjpegFile;
File audioFile;
boolean firsttime = true;
Audio audio;

boolean readyForNextVideo = true;
boolean readyForNextAudio = true;

static int total_frames = 0;
static unsigned long total_read_video = 0;
static unsigned long total_decode_video = 0;
static unsigned long total_show_video = 0;
static unsigned long start_ms, curr_ms;

long timer = 0;

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long start = millis();
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  total_show_video += millis() - start;
  return 1;
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);  
  SPI.setFrequency(8000000);

  Serial.begin(115200);
  smartdelay(1000);
  Serial.println("MJPEG Movie and Audio Player");

  if ( ! SD.begin(SD_CS) )
  {
    Serial.println(F("SD card failed"));
    while(1);
  }
  else
  {
    Serial.println(F("SD card mounted"));
  }

  smartdelay(1000);

  audio.setPinout( I2S_BCLK /* bclkPin */, I2S_LRC /* wclkPin */, I2S_DOUT /* doutPin */);
  audio.setVolume(15); // 0...21
    
  mjpeg_buf = (uint8_t *) malloc(MJPEG_BUFFER_SIZE);
  if (!mjpeg_buf)
  {
    Serial.println(F("mjpeg_buf malloc failed"));
    while(1);
  }

  smartdelay(1000);

  // Init Display
  gfx->begin();
  gfx->fillScreen(YELLOW);
  gfx->invertDisplay(true);
  gfx->setRotation(2);  

  timer = millis() + ( 1000 / 15 );
}

/*
 * Plays all the .mjpeg files in the root directory of the SD card
 */

int t2 = 5;

void playVideo()
{    
  if ( videoDir == NULL )
  {
    videoDir = SD.open("/");
  }

  if ( readyForNextVideo )
  {            
    mjpegFile = videoDir.openNextFile();

    if ( mjpegFile ) 
    {
      if ( ( String(mjpegFile.name()).endsWith(".mjpeg") ) 
        && ( ! mjpegFile.isDirectory() ) 
        && ( ! String(mjpegFile.name()).startsWith(".") )
        )
      {
        Serial.printf_P(PSTR("MJPEG start '%s' from SD card...\n"), mjpegFile.name());  
        start_ms = millis();
        curr_ms = millis();
        mjpeg.setup(
            &mjpegFile, mjpeg_buf, jpegDrawCallback, true /* useBigEndian */,
            0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */, firsttime);
        firsttime = false;
        readyForNextVideo = false;
        return;
      }
      return;
    }
    else
    {
      Serial.println(F("End of root directory"));
      videoDir.close();
      readyForNextVideo = true;
      return;
    }
  }
  else
  {
    if ( mjpegFile.available() )
    {
      // Read video

      if ( t2-- == 0 )
      { 
        t2 = 4;
              
        mjpeg.readMjpegBuf();
        total_read_video += millis() - curr_ms;
        curr_ms = millis();
  
        // Play video
        mjpeg.drawJpg();
        total_decode_video += millis() - curr_ms;
    
        curr_ms = millis();
        total_frames++;
      }
    }
    else
    {
      mjpegFile.close();
      readyForNextVideo = true;

      int time_used = millis() - start_ms;
      Serial.println(F("MJPEG end"));
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

/*
 * Plays all the .mjpeg files in the root directory of the SD card
 */

void playAudio()
{
  if ( audioDir == NULL )
  {
    audioDir = SD.open("/");
  }  

  if ( readyForNextAudio )
  {            
    audioFile = audioDir.openNextFile();
    if ( audioFile ) 
    {      
      if ( 
        ( String(audioFile.name()).endsWith(".m4a") ) 
        && ( ! audioFile.isDirectory() ) 
        && ( ! String(audioFile.name()).startsWith("/._") ) 
      ) 
      {
        Serial.printf_P(PSTR("Found audio '%s' from SD card\n"), audioFile.name());
        audio.connecttoFS(SD, audioFile.name() );
        readyForNextAudio = false;
      }
      return;
    }
    else
    {
      Serial.println(F("End of root directory for audio"));
      audioDir.close();
      readyForNextAudio = true;
      return;
    }
  }
  else
  {
    audio.loop();
  }
}

// Audio system interrupt callbacks

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.println( "Audio ended" );
    Serial.println(info);
    readyForNextAudio = true;
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}

int t = 3;

void loop()
{
/*
  if ( t-- == 0 )
  { 
    playVideo();
    t = 4;
  }
  */
  
  playVideo();
  playAudio();
}
