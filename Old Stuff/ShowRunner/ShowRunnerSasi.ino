/*
 * Decodes a Show archive containing audio/video media and show running instructions
 * on the Seuss display board
 * 
 * Archive is in compressed TAR format
 * Show running instructions are in a JSON formatted file with .REF extension
 * Audio media is WAV formatted sampled at 8000 Khz
 * Video media is MJEPG at 240x240 pixel uncompressed
 *
 * Board wiring directions and code at https://github.com/frankcohen/ReflectionsOS
 *
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * September 5, 2021

   Uses I2S audio player library from
   https://github.com/schreibfaul1/ESP32-audioI2S

   Using Arduino_GFX class by @moononournation
   https://github.com/moononournation/Arduino_GFX
   For speed and support of MPEG video

   Note: To play 240x240 MJPEG uncompressed files requires the audio at
   a sample rate of 8000 kHz, 16 bits, stereo format, WAVE format

   Using ESP32-targz library to uncompress tar files
   https://github.com/tobozo/ESP32-targz

   Using L

   Thanks to the ArduinoJSON project for a well implemented library
   https://arduinojson.org/v6/example/
   Developer journal on using ArduinoJson is at https://bit.ly/3rxXIZX

 */

#include "secrets.h"    // Contains IDs and passwords, excluded from repository, see secrets.h.example.h
#include "settings.h"
#include "Utils.h"
#include "Reflections.h"

void setup() {
  Serial.begin(115200);
  smartDelay(2000);

  Serial.println("Reflections ShowRunner");
  
  initWiFi(SECRET_SSID, SECRET_PASSWORD);
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  enableOneSPI( DisplayCS );

  initDisplay();

  //initBacklight();

  enableOneSPI( DisplaySDCS );
  
  if ( !SD.begin( DisplaySDCS, SPI, 80000000 ) ) /*    SPI bus mode */
  {
    SDfailedInit();
  }
  else {
    sdCardValid = true;    
  }

  //findOneFile();
  //getFileSaveToSDCard();

  //extractTar();

  enableOneSPI( DisplaySDCS );

  parseJson();
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);

  InitializeAudioTask();

  //Startup Sequence
  playMedia("/DemoReel3", onStartVID, onStartAUD);

}

void loop() {

  for(int i = 0; i < numEvents; i++){
    for(int j = 0; j < numSeq[i]; j++) {
/*
      if(interrupted1){
        playMedia("/DemoReel3", ButtonOnePressedVID, ButtonOnePressedAUD);
        interrupted1 = false;
      } else if(interrupted3){
        playMedia("/DemoReel3", ButtonThreePressedVID, ButtonThreePressedAUD);
        interrupted3 = false;
      } else if(interruptedT){
        playMedia("/DemoReel3", onHourVID, onHourAUD);
      }
*/
      playMedia("/DemoReel3", EventVID[i][j], EventAUD[i][j]);
    }
  }

}
