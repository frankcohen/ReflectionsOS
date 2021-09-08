/*
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * September 5, 2021
*/

#ifndef utils
#define utils

#include "HTTPClient.h"
#include <SD.h>
#include "settings.h"

String fileName = "";   // this is the file name received from the server

void disableSPIDevice( int deviceCS )
{
    //Serial.print( F("Disabling SPI device on pin ") );
    //Serial.println( deviceCS );
    pinMode(deviceCS, OUTPUT);
    digitalWrite(deviceCS, HIGH);
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

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    // feel free to do something here
  } while (millis() - start < ms);
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

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

boolean findOneFile()
{
        Serial.println( "findOneFile" );

        HTTPClient http;

        http.begin(serverDomain, serverPort, "/onefilename");

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                //Serial.print( "Server response code: " );
                //Serial.println( httpCode );
                return false;
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        // Serial.print( "Size: " );
        // Serial.println( len );

        fileName = http.getString();
        if ( !fileName.equals( "nofiles" ) )
        {
                Serial.print( "fileName: " );
                Serial.println( fileName );
        }

        http.end();
        return true;
}

boolean getFileSaveToSDCard()
{
        HTTPClient http;

        http.begin(serverDomain, serverPort, "/download?file=" + fileName);

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                Serial.print( "Server response code: " );
                Serial.println( httpCode );
                return false;
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        Serial.print( "Size: " );
        Serial.println( len );

        // create buffer for read
        uint8_t buff[130] = { 0 };

        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        enableOneSPI( DisplaySDCS );

        File myFile = SD.open( "/" + fileName, "wb" );
        if ( myFile )
        {
                Serial.print( "SD file opened for write: " );
                Serial.println( fileName );
        }
        else
        {
                Serial.print( "Error opening new file for writing: " );
                Serial.println( fileName );
                return false;
        }

        int startTime = millis();
        int bytesReceived = 0;
        boolean buffirst = true;

        // read data from server
        while( http.connected() && (len > 0 || len == -1))
        {
          size_t size = stream->available();
          byte swap;
          if(size) 
          {
              int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size) );
              bytesReceived += c;

              // Use in case of endian problem
              /*
              byte swapper = 0;
              for ( int n = 0; n<130; n = n+2 )
              {
                swapper = buff[n];
                buff[n] = buff[n+1];
                buff[n+1] = swapper;
              }
              */
                            
              myFile.write( buff, c );
              if(len > 0) 
              {
                      len -= c;
              }
          }
          smartDelay(1);
        }

        myFile.close();
        http.end();

        Serial.print( "Bytes received " );
        Serial.print( bytesReceived );
        Serial.print( " in " );
        Serial.print( ( millis() - startTime ) / 1000 );
        Serial.print( " seconds " );
        if ( ( ( millis() - startTime ) / 1000 ) > 0 )
        {
                Serial.print( bytesReceived / ( ( millis() - startTime ) / 1000 ) );
                Serial.print( " bytes/second" );
        }
        Serial.println( " " );

        return true;
}


boolean deleteFileFromServer()
{
        Serial.println( "deleteFileFromServer" );

        HTTPClient http;

        http.begin(serverDomain, serverPort, "/delete?file=" + fileName );

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                Serial.print( "Server response code: " );
                Serial.println( httpCode );
                return false;
        }

        http.end();
        return true;
}




void removeFiles(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Removing files from directory: %s\n", dirname);

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

            deleteFile(SD, file.name() );
        }
        file = root.openNextFile();
    }
}

void printDirectory(File dir, int numTabs) 
{
        while (true)
        {
                File entry =  dir.openNextFile();
                if (!entry) {
                        // no more files
                        break;
                }

                for (uint8_t i = 0; i < numTabs; i++) {
                        Serial.print('\t');
                }

                Serial.print(entry.name());

                if (entry.isDirectory()) {
                        Serial.println("/");
                        printDirectory(entry, numTabs + 1);
                } else {
                        // files have sizes, directories do not
                        Serial.print("\t\t");
                        Serial.println(entry.size(), DEC);
                }
                entry.close();
        }
}

#endif
