/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
*/

#include "Utils.h"

Utils::Utils(){}

void Utils::begin()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  pinMode(SPI_DisplayCS, OUTPUT);
  digitalWrite(SPI_DisplayCS, HIGH);

  pinMode(SPI_DisplayDC, OUTPUT);
  digitalWrite(SPI_DisplayDC, HIGH);

  pinMode(SPI_DisplayRST, OUTPUT);
  digitalWrite(SPI_DisplayRST, HIGH);

  pinMode(SPI_MISO,INPUT_PULLUP);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_SCK, OUTPUT);

  Serial.println("Starting SPI");
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(8000000);  // at 8000000 I get CPU panic reboots

  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

void Utils::loop()
{
}

void Utils::startSDWifi()
{
  if (!SD.begin( SD_CS )) 
  {
    Serial.println(F("Storage initialization failed"));
  } else {
    Serial.println(F("Storage initialization success"));
  }
  
  _wifiMulti.addAP( wifiSSID, wifiPass );

  Serial.println("Connecting Wifi");

  long ago = millis();
  while ( millis() < ( ago + ( 1000*10 ) ) )
  {
    if(_wifiMulti.run() == WL_CONNECTED) {
        //Serial.println("WiFi connected");
        break;
    }
  }
  if ( _wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
  }  
}

void Utils::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
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
                listDir(fs, file.path(), levels -1);
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
