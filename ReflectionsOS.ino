/*
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Wiring, repository and community discussions at https://github.com/frankcohen/ReflectionsOS
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

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Arduino_GFX.h>

// #include <Arduino_ST7735_STM.h> // Hardware-specific library for ST7789, https://github.com/cbm80amiga/Arduino_ST7735_STM
#include <Adafruit_ST7789.h>

#define SerialSpeed 115200  // Serial monitor speed

// Chip Select for devices on the SPI bus, the following are
// the GPIO numbers (not the pin numbers)

#define DisplayCS 32     //TFT display on Adafruit's ST7789 card
#define DisplaySDCS 4    //SD card on the Adafruit ST7789 card
#define DisplayRST 17    //Reset for Adafruit's ST7789 card
#define DisplayDC 16     //DC for Adafruit's ST7789 card

#define SDcardCS 4       //MicroSD memory card on Adafruit's Micro SD Breakout Board

// Devices attached to VSPI ON ESP32
#define SPIMOSI 23
#define SPIMISO 19
#define SPICLK 18

// Additional devices and buses
// GPS
#define GPSTX 1
#define GPSRX 3

// MEMS accelerometer, magnetometer and gyroscope, I2C bus
#define CompassSCL 22
#define CompassSDA 21

#define SCR_WD 240
#define SCR_HT 240

Adafruit_ST7789 lcd = Adafruit_ST7789(DisplayCS, DisplayDC, SPIMOSI, SPICLK, DisplayRST);

//Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SPICLK, SPIMOSI, SPIMISO);
//Arduino_ST7789 *lcd = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);

void setup() 
{
  Serial.begin( SerialSpeed );

  Serial.println( F("Reflections: Seuss Display" ) );
  Serial.println( F("Frank Cohen, fcohen@votsh.com, January 1, 2021") );
  Serial.println( F("GPL v3 license" ) );
  Serial.println( F("https://github.com/frankcohen/ReflectionsOS") );
  
  enableOneSPI( SDcardCS );

  if (!SD.begin( SDcardCS )) {
    Serial.println("SD begin failed");
  }

  Serial.println("Showing video");
  showVideo("StartupR.mov", 0, 0, 200, 120, 30, 1); 

  Serial.println("Setup complete");
}

/*
 * Configure SPI bus to send data to the display
 */
 
void lcdSPI()
{
  SPI.beginTransaction(SPISettings(36000000, MSBFIRST, SPI_MODE3));
}

/*
 * Enable the fash bus to move data
 */
#define SD_SPEED 36
#define SD_SCK_MHZ(maxMhz) (1000000UL*(maxMhz))

void sdSPI()
{
  SPI.beginTransaction( SPISettings( SD_SCK_MHZ(SD_SPEED), MSBFIRST, SPI_MODE3) );
}

/*
 * Play video from SD card to display
 */
 
#define NLINES 200
uint16_t buf[200*NLINES]; 
char txt[30];
int statMode=0, prevStat=0;

// Params:
// name - file name
// x,y - start x,y on the screen
// wd,ht - width, height of the video (raw data has no header with such info)
// nl - num lines read in one operation (nl*wd*2 bytes are loaded)
// skipFr - num frames to skip

void showVideo(char *name, int x, int y, int wd, int ht, int nl, int skipFr)
{
  File file;

  lcd.setTextColor(YELLOW,BLACK);
  
  file = SD.open(name, FILE_READ);
  if ( !file )
  {
    lcdSPI(); lcd.fillScreen(YELLOW);
    Serial.println( F("File open failed") );
  }
  
  while(file.available()) 
  {
    for(int i=0;i<ht/nl;i++) {
      
      int rd = file.read( (uint8_t *) buf, (long) wd*2*nl);
      lcdSPI();
      for(int j=0;j<nl;j++)
      {
        lcd.drawImage(0,i*nl+j+(statMode>0?0:4),lcd.width(),1,buf+20+j*wd);
      }
    }
  }

  file.close();
}

/*
 * List files from SD card to the serial monitor
 */

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
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
  if ( deviceCS != DisplaySDCS ) { disableSPIDevice( DisplaySDCS ); }
  if ( deviceCS != DisplayRST ) { disableSPIDevice( DisplayRST ); }
  if ( deviceCS != DisplayDC ) { disableSPIDevice( DisplayDC ); }
/*
  if ( deviceCS != AudioCS ) { disableSPIDevice( AudioCS ); }
  if ( deviceCS != AudioXDCS ) { disableSPIDevice( AudioXDCS ); }
  if ( deviceCS != AudioSDCS ) { disableSPIDevice( AudioSDCS ); }
  if ( deviceCS != AudioRST ) { disableSPIDevice( AudioRST ); }
*/
  if ( deviceCS != SDcardCS ) { disableSPIDevice( SDcardCS ); }

  Serial.print( F("\nEnabling SPI device on pin ") );
  Serial.println( deviceCS );

  pinMode(deviceCS, OUTPUT);
  digitalWrite(deviceCS, LOW);
}

void disableSPIDevice( int deviceCS )
{
    Serial.print( F("\nDisabling SPI device on pin ") );
    Serial.println( deviceCS );
    pinMode(deviceCS, OUTPUT);
    digitalWrite(deviceCS, HIGH);
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
    playMusicTrack( "/track001.mp3" );

  Serial.println( F("\nRepeat?\n") );
  while (!Serial.available()) {
    SysCall::yield();
  }
*/


}
