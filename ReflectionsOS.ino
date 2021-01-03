/*
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Licensed under GPL v3 
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * January 2, 2021
 */

/*
Using the ESP32 development board from Hiletgo
http://www.hiletgo.com/ProductDetail/1906566.html
ESP-WROOM-32 ESP32 ESP-32S Development Board 2.4GHz Dual-Mode WiFi + Bluetooth Dual Cores Microcontroller

This sketch uses the F notation to store strings in flash to save RAM

Thank you Pawel A. Hernik (https://youtu.be/o3AqITHf0mo) for tips on how to drive raw/uncompressed
video to the display.
*/

SdFs sd;
cid_t m_cid;
csd_t m_csd;
uint32_t m_eraseSize;
uint32_t m_ocr;
static ArduinoOutStream cout(Serial);

#define SerialSpeed 115200  // Serial monitor speed

// Chip Select for devices on the SPI bus, the following are
// the GPIO numbers (not the pin numbers)

#define DisplayCS 33      //TFT display on Adafruit's ST7789 Card
#define AudioCS 16        //Adafruit's vs1053 Breakout Board
#define AudioXDCS 17      //vs1053 chip on Adafruit's Breakout Board
#define AudioSDCardCS 32  //MicroSD memory card on Adafruit's vs1053 Breakout Board
#define SDcardCS 27       //MicroSD memory card on Adafruit's Micro SD Breakout Board

// Devices attached to VSPI ON ESP32
#define SPIMOSI 23
#define SPIMISO 19
#define SPICLK 18

// Audio player/recorder VS1053
#define AudioReset 25
#define AudioDREQ 4

// Additional devices and buses
// GPS, RX/TX bus
// MEMS accelerometer, magnetometer and gyroscope, I2C bus

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  Adafruit_VS1053_FilePlayer(AudioReset, AudioCS, AudioXDCS, AudioDREQ, AudioSDCardCS);


void setup() {
  
  Serial.begin( SerialSpeed );
  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }

  Serial.println( F("Reflections: Seuss Display" ));
  Serial.println( F("Frank Cohen, fcohen@votsh.com, January 1, 2021"));
  Serial.println( F("GPL v3 license" ));
  
  //enableOneSPI( AudioCS );
  disableSPIDevice( DisplayCS );
  disableSPIDevice( AudioCS );
  disableSPIDevice( AudioXDCS );
  disableSPIDevice( AudioSDCardCS );
  disableSPIDevice( SDcardCS );
}

void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}

/*
 * Disable all the SPI devices and enable one of them
 */
 
void enableOneSPI( int deviceCS )
{
  if ( deviceCS != DisplayCS ) { disableSPIDevice( DisplayCS ); }
  if ( deviceCS != AudioCS ) { disableSPIDevice( AudioCS ); }
  if ( deviceCS != AudioXDCS ) { disableSPIDevice( AudioXDCS ); }
  if ( deviceCS != AudioSDCardCS ) { disableSPIDevice( AudioSDCardCS ); }
  if ( deviceCS != SDcardCS ) { disableSPIDevice( SDcardCS ); }  

  pinMode(deviceCS, OUTPUT);
  digitalWrite(deviceCS, LOW);
}

void disableSPIDevice( int deviceCS )
{
    // Serial.print( F("\nDisabling SPI device on pin ") );
    // Serial.println( deviceCS );
    pinMode(deviceCS, OUTPUT);
    digitalWrite(deviceCS, HIGH);
}

void errorPrint() {
  if (sd.sdErrorCode()) {
    cout << F("SD errorCode: ") << hex << showbase;
    printSdErrorSymbol(&Serial, sd.sdErrorCode());
    cout << F(" = ") << int(sd.sdErrorCode()) << endl;
    cout << F("SD errorData = ") << int(sd.sdErrorData()) << endl;
  }
}

void printCardLS()
{ 
  Serial.println( F("\nList of files on the SD card\n") );
    if ( !sd.ls("/", LS_SIZE | LS_R ) ) {
      Serial.println( "List failed");    
      errorPrint();
    }
}

#define MusicCS 16
#define MusicXDCS 17
#define MusicSDCS 32
#define MusicRST 25
#define MusicDREQ 4

#define SPICLK 18
#define SPIMOSI 23
#define SPIMISO 19
#define SPIRST 25

void playMusicTrack( String trackname )
{
  Adafruit_VS1053_FilePlayer musicPlayer = 
      Adafruit_VS1053_FilePlayer( SPIMOSI, SPIMISO, SPICLK, SPIRST, 
      MusicCS, MusicXDCS, MusicDREQ, MusicSDCS );
     
  if (! musicPlayer.begin() )
  {
    Serial.println( "Music player did not initialize" );
    return;        
  }

  musicPlayer.setVolume(20,20);

  musicPlayer.sineTest(0x44, 500); // Make a tone to indicate VS1053 is working 
  
  musicPlayer.playFullFile( "/track003.ogg" );
  
  delay(5000);
}

void loop() {

  /*
  clearSerialInput();

  uint32_t t = millis();
  if (!sd.begin( SdSpiConfig( AudioSDCardCS, DEDICATED_SPI, SD_SCK_MHZ(16)) )) {
    Serial.println( "SD initialization failed");    
    errorPrint();
  }

  t = millis() - t;
  cout << F("init time: ") << t << " ms" << endl;

  if (!sd.card()->readCID(&m_cid) ||
      !sd.card()->readCSD(&m_csd) ||
      !sd.card()->readOCR(&m_ocr)) {
    cout << F("readInfo failed\n");
    errorPrint();
  }
  else
  {
    // printCardType();
    // printCardLS();
    playMusicTrack( "/track001.mp3" );
    
  }  
*/
    playMusicTrack( "/track001.mp3" );

  Serial.println( F("\nRepeat?\n") );
  while (!Serial.available()) {
    SysCall::yield();
  }

}
