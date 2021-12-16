#define DEST_FS_USES_SD
#include <SD.h>
#include <Arduino_GFX_Library.h>
#include <ArduinoJson.h>
#include <ESP32-targz.h>
#include "Audio.h"
#include "FS.h"
#include "Arduino.h"
#include <WiFi.h>
#include <wifiMulti.h>
#include <HTTPClient.h>

#define BUTTON_LEFT   36
#define BUTTON_RIGHT  39

WiFiMulti wifiMulti;
bool endFlag = false;
#define MJPEG_FILENAME ".mjpeg"
#define MJPEG_BUFFER_SIZE (240 * 240 / 3)

#define TFT_BRIGHTNESS 250  // Hearing static over the I2S speaker when brightness > 200

#define SCK     18
#define MOSI    23
#define MISO    19
#define SS      5

#define DisplayCS   5   //TFT display on Adafruit's ST7789 card
#define DisplaySDCS 4    //SD card on the Adafruit ST7789 card
#define DisplayRST  17    //Reset for Adafruit's ST7789 card
#define DisplayDC   16   //DC for Adafruit's ST7789 card

#define I2S_DOUT      33
#define I2S_BCLK      26
#define I2S_LRC       25

// ST7789 Display
Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);

Audio audio(true, I2S_DAC_CHANNEL_LEFT_EN);
TaskHandle_t AudioTaskHandle;
bool notify = false;
bool syncFlag = false;
char message[300];
char onStartVID[30];
char onStartAUD[30];
char onHourVID[50];
char onHourAUD[50];
char ButtonOnePressedVID[30];
char ButtonOnePressedAUD[30];
char ButtonThreePressedVID[30];
char ButtonThreePressedAUD[30];
char EventVID[5][5][30];          //Index 1-> Event index; Index 2-> Sequence index; Index 3 -> String index
char EventAUD[5][5][30];
int  numSeq[] = {0, 0, 0, 0, 0};  //Number of sequences
int  numEvents = 0;               //Number of event type = event
bool test_succeeded = false;
const char* ssid                = "GNXS-2.4G-36F1E4";
const char* password            = "mangalam123";
const char* ntpServer           = "pool.ntp.org";
const long  gmtOffset_sec       = 19800;
const int   daylightOffset_sec  = 0;


#include "MjpegClass.h"
static MjpegClass mjpeg;
boolean sdCardValid = false;
boolean firstTime = true;
uint8_t *mjpeg_buf;
File dir;
bool interrupted1 = false;
bool interrupted3 = false;
bool interruptedT = false;

void AudioTask( void * pvParameters ) {
    //audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    //audio.setVolume(21);
    audio.connecttoFS(SD, "/DemoReel3/startup.m4a");
      
  for(;;){
    
    if(!notify && (audio.getAudioCurrentTime() <= audio.getAudioFileDuration()-2) && !endFlag){
      audio.loop();
      
    } else if (notify) {
      audio.stopSong();
      audio.connecttoFS(SD, message);
      audio.loop();
      notify = false; 
      endFlag = false;
      syncFlag = true;
    } else if(!endFlag && syncFlag) {
      Serial.println("Audio Task");
      endFlag = true;
      syncFlag = false;
      delay(1);
    } else {
      delay(1);
    }
    
  }
}

void audio_info(const char *info){
     Serial.print("info        "); Serial.println(info);
 }


bool test_tarExpander() {
  bool ret = false;
  const char* tarFile = "/DemoReel3.tar";

  Serial.printf("Testing tarExpander\n");

  TarUnpacker *TARUnpacker = new TarUnpacker();

  TARUnpacker->haltOnError( true ); // stop on fail (manual restart/reset required)
  TARUnpacker->setTarVerify( true ); // true = enables health checks but slows down the overall process

  TARUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
  TARUnpacker->setLoggerCallback( BaseUnpacker::targzPrintLoggerCallback  );  // log verbosity
  TARUnpacker->setTarProgressCallback( BaseUnpacker::defaultProgressCallback ); // prints the untarring progress for each individual file
  TARUnpacker->setTarStatusProgressCallback( BaseUnpacker::defaultTarStatusProgressCallback ); // print the filenames as they're expanded
  TARUnpacker->setTarMessageCallback( BaseUnpacker::targzPrintLoggerCallback ); // tar log verbosity

  if(  !TARUnpacker->tarExpander(tarGzFS, tarFile, tarGzFS, "/") ) {
    Serial.printf("tarExpander failed with return code #%d\n", TARUnpacker->tarGzGetError() );
  } else {
    ret = true;
  }
  return ret;
} 

void disableSPIDevice( int deviceCS )
{
    //Serial.print( F("Disabling SPI device on pin ") );
    //Serial.println( deviceCS );
    pinMode(deviceCS, OUTPUT);
    digitalWrite(deviceCS, HIGH);
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    // feel free to do something here
  } while (millis() - start < ms);
}

void initWiFi(const char* _ssid, const char* _pswd) {
  Serial.printf("Connecting to %s ", _ssid);
  wifiMulti.addAP(_ssid, _pswd);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      wifiMulti.run();
  }
  Serial.println("CONNECTED");
}

void initDisplay() {
  gfx->begin();
  gfx->fillScreen(YELLOW);
}

void extractTar() {
  Serial.println("EXTRACTING FILE");

  if (!tarGzFS.begin()) {
    Serial.printf("%s Mount Failed, halting\n", FS_NAME );
    while(1) yield();
  } else {
    Serial.printf("%s Mount Successful\n", FS_NAME);
  }

  test_succeeded = test_tarExpander();

  if( test_succeeded ) {
    Serial.printf("FILES EXTRACTED\n");
  }
}

void SDfailedInit() {
  Serial.print( "SD card on GPIO " );
  Serial.print( DisplaySDCS );
  Serial.println( " failed to start"); 
  gfx->println(F("ERROR: SD card mount failed"));
  gfx->fillScreen(BLUE);    
  while(1);
}

void parseJson() {
  File file = SD.open("/DemoReel3/DemoReel3.json");
  Serial.println("Deserializing JSON");
  DynamicJsonDocument doc(2048);
  auto error = deserializeJson(doc, file);

  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  JsonVariant title = doc.getMember("ReflectionsShow").getMember("title");

  if ( title.isNull() ) {
    Serial.println( "Title not found");
  }

  JsonObject events = doc["ReflectionsShow"]["events"];

  if ( events.isNull() ) {
    Serial.println( "Events not found");
  }

  for (auto eventseq : events)  {
    String eventskey = eventseq.key().c_str();
    String eventsvalue = eventseq.value().as<const char*>();  //ver 6.18
    JsonObject event = doc["ReflectionsShow"]["events"][ eventskey ];
    String eventname = event["name"].as<char*>();
    String eventtype = event["type"].as<char*>();
    JsonArray sequence = doc["ReflectionsShow"]["events"][eventskey]["sequence"];

    if(eventtype == "OnStart") {

      sprintf(onStartVID, "%s", sequence[0]["playvideo"].as<char*>());
      Serial.printf("OnStartVideo: %s\n", onStartVID);

      sprintf(onStartAUD, "%s", sequence[0]["playaudio"].as<char*>());
      Serial.printf("OnStartAudio: %s\n", onStartAUD);

    } else if(eventtype == "OnHour") {

      sprintf(onHourVID, "%s", sequence[0]["playvideo"].as<char*>());
      Serial.printf("OnHourVideo: %s\n", onHourVID);

      sprintf(onHourAUD, "%s", sequence[0]["playaudio"].as<char*>());
      Serial.printf("OnHourAudio: %s\n", onHourAUD);

    } else if(eventtype == "ButtonOnePressed") {

      sprintf(ButtonOnePressedVID, "%s", sequence[0]["playvideo"].as<char*>());
      Serial.printf("ButtonOnePressedVideo: %s\n", ButtonOnePressedVID);

      sprintf(ButtonOnePressedAUD, "%s", sequence[0]["playaudio"].as<char*>());
      Serial.printf("ButtonOnePressedAudio: %s\n", ButtonOnePressedAUD);

    } else if(eventtype == "ButtonThreePressed") {

      sprintf(ButtonThreePressedVID, "%s", sequence[0]["playvideo"].as<char*>());
      Serial.printf("ButtonThreePressedVideo: %s\n", ButtonThreePressedVID);

      sprintf(ButtonThreePressedAUD, "%s", sequence[0]["playaudio"].as<char*>());
      Serial.printf("ButtonThreePressedAudio: %s\n", ButtonThreePressedAUD);

    } else if(eventtype == "event") {

      int seqIndex = 0;
      for (JsonObject step : sequence) {
        if(step["playaudio"]){
          sprintf(EventAUD[numEvents][seqIndex], "%s", step["playaudio"].as<char*>());
          Serial.printf("Event %d Sequence %d Audio: %s\n", numEvents, seqIndex, EventAUD[numEvents][seqIndex]);
        } else {
          sprintf(EventAUD[numEvents][seqIndex], "~Continue~");
        }

        if(step["playvideo"]){
          sprintf(EventVID[numEvents][seqIndex++], "%s", step["playvideo"].as<char*>());
          Serial.printf("Event %d Sequence %d Video: %s\n", numEvents, seqIndex - 1, EventVID[numEvents][seqIndex - 1]);
        }
      }
      numSeq[numEvents] = seqIndex;
      numEvents++;
    }
  }

  Serial.println("done");
  file.close();
}
//
void InitializeAudioTask() {
  xTaskCreatePinnedToCore(
                    AudioTask,   /* Task function. */
                    "AudioTask", /* name of task. */
                    3500,      /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &AudioTaskHandle,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */                  
  delay(500);
  Serial.println("Task created");
}

bool keepAlive(struct tm timeinfo) {

  if(digitalRead(BUTTON_LEFT) == HIGH){
    delay(500);
    Serial.println(timeinfo.tm_min);
    interrupted1 = true;
    return true;
  } else if(digitalRead(BUTTON_RIGHT) == HIGH){
    delay(500);
    interrupted3 = true;
    return true;
  } else if(timeinfo.tm_min == 0 && !interruptedT){
    delay(500);
    interruptedT = true;
    return true;
  } else if(timeinfo.tm_min != 0){
    interruptedT = false;
  }
  return false;
}

/*
 * Streams an mjpeg file from an SDRAM card to the display
 */
 
void streamVideo( File vFile ) {


  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  Serial.print("streamVideo for ");
  Serial.println( vFile.name() );

  if (!vFile || vFile.isDirectory()) {
    Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
    gfx->println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
  } else {
    Serial.print( "Opened " );
    Serial.println( vFile.name() );

    if ( firstTime ) {
      mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
      mjpeg.setup(vFile /* file */, mjpeg_buf /* buffer */, gfx /* driver */, true /* multitask */, firstTime );
      firstTime = false;
    } else {
      mjpeg.setup(vFile /* file */, mjpeg_buf /* buffer */, gfx /* driver */, true /* multitask */, firstTime );
    }
    
    if (!mjpeg_buf) {
      Serial.println(F("mjpeg_buf malloc failed!"));
    } else {
      // Stream video to display

      while (mjpeg.readMjpegBuf()) {
        // Play video
        mjpeg.drawJpg();
        
        if(endFlag) {
          endFlag = false;
          vFile.close();
          break;
        }

        if(keepAlive(timeinfo)) {
          vFile.close();
          break;
        }
        
      }

      Serial.println(F("MJPEG video end"));
      vFile.close();
    }
  }
}

void playMedia(char* destination, char* videoFile, char* audioFile = "SpareMe.m4a") {
  dir = SD.open(destination);
  char videoNamebuff[100];
  char audioNamebuff[100];
  sprintf(videoNamebuff, "%s/%s", destination, videoFile);
  sprintf(audioNamebuff, "%s/%s", destination, audioFile);

  if ( ! dir ) {
    Serial.println( "No result for opening the directory" );
  } else {
    File videoFile = SD.open(videoNamebuff);

    if(strcmp(audioFile, "~Continue~")) {
      sprintf(message, audioNamebuff);
      notify = true;
    }

    streamVideo(videoFile);

  }
}

void enableOneSPI( int deviceCS ) {
  if ( deviceCS != DisplayCS ) { disableSPIDevice( DisplayCS ); }
  if ( deviceCS != DisplaySDCS ) { disableSPIDevice( DisplaySDCS ); }
  if ( deviceCS != DisplayRST ) { disableSPIDevice( DisplayRST ); }
  if ( deviceCS != DisplayDC ) { disableSPIDevice( DisplayDC ); }
  //Serial.print( F("Enabling SPI device on GPIO ") );
  //Serial.println( deviceCS );

  pinMode(deviceCS, OUTPUT);
  digitalWrite(deviceCS, LOW);
}
