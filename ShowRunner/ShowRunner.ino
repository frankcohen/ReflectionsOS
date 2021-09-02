#include "Reflections.h"

void setup() {
  Serial.begin(115200);

  initWiFi(ssid, password);
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.println("SD MJPEG Video");
  
  enableOneSPI( DisplayCS );

  initDisplay();
  
  //extractTar();

  enableOneSPI( DisplaySDCS );

  if (!SD.begin(/*DisplaySDCS, SPI, 80000000*/)) /* SPI bus mode */{
    SDfailedInit();
  }
  else {
    sdCardValid = true;    
  }

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

      if(interrupted1){
        playMedia("/DemoReel3", ButtonOnePressedVID, ButtonOnePressedAUD);
        interrupted1 = false;
      } else if(interrupted3){
        playMedia("/DemoReel3", ButtonThreePressedVID, ButtonThreePressedAUD);
        interrupted3 = false;
      } else if(interruptedT){
        playMedia("/DemoReel3", onHourVID, onHourAUD);
      }

      playMedia("/DemoReel3", EventVID[i][j]);
    }
  }

}
