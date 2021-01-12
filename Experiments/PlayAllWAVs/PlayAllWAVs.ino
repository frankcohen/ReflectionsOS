#include <Arduino.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// For this sketch, you need connected SD card with '.wav' music files in the root
// directory. 

#define SPI_SPEED SD_SCK_MHZ(40)

File dir;
AudioFileSourceSD *source = NULL;
AudioGeneratorWAV *wav = NULL;
AudioOutputI2S *output;

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  audioLogger = &Serial;  
  source = new AudioFileSourceSD();
  output = new AudioOutputI2S();
  wav = new AudioGeneratorWAV();

  // NOTE: SD.begin(...) should be called AFTER AudioOutputSPDIF() 
  //       to takover the the SPI pins if they share some with I2S
  //       (i.e. D8 on Wemos D1 mini is both I2S BCK and SPI SS)
  if ( !SD.begin( 4 ) )
  {
    Serial.println( "Serial card begin failed" );
    while(1);
  }
  dir = SD.open("/"); 
}

void loop() {
  if ((wav) && (wav->isRunning())) {
    if (!wav->loop()) wav->stop();
  } 
  else 
  {
    File file = dir.openNextFile();
    if (file) {      
      if (String(file.name()).endsWith(".wav")) {
        source->close();
        if (source->open(file.name())) { 
          Serial.printf_P(PSTR("Playing '%s' from SD card...\n"), file.name());
          wav->begin(source, output);
        } else {
          Serial.printf_P(PSTR("Error opening '%s'\n"), file.name());
        }
      } 
    } else {
      Serial.println(F("Playback form SD card done\n"));
      delay(1000);
    }       
  }
}
