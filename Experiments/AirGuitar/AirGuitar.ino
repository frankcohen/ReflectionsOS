/**
 * @file AirGuitar.ino
 * @author Frank Cohen, fcohen@starlingwatch.com
 * @brief Plays music over Bluetooth Classic A2DP protocol to portable speaker
 * 
 * @version 0.1
 * @date 2023-05-17
 * 
 * @copyright Licensed under GPL v3 Open Source Software, (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com, Read the license in the license.txt file that comes with this code.
 * 
 * Depends on:
 *   Arduino-Audio-Tools library 
 *   https://github.com/pschatzmann/arduino-audio-tools
 *   ESP32-A2DP library
 *   https://github.com/pschatzmann/ESP32-A2DP
 *   and
 *   https://github.com/pschatzmann/arduino-libhelix
 *   Accelerometer
 *   https://github.com/adafruit/Adafruit_BNO055
 *   
 * See AirGuitar_Notes.md for notes, schematics, 
 *   
 *   May 13, 2023
fcohen@starlingwatch.com

Reflections Air Guitar

Plays audio as board is moved up-and-down as an "air guitar"
Plays .mp3 audio files over Bluetooth A2DP to portable speakers
Requires Bluetooth Classic, so yes to ESP32, and no to ESP32S3 (BLE)

![AirGuitar breadboard](AirGuitar.jpg "Components and assembly instructions")

Components
ESP32 Dev Board from HiLetGo.com
Adafruit NAND SD SPI SMT card
Adafruit BN055 9 axis accelerometer

Depends on
Arduino-Audio-Tools library 
https://github.com/pschatzmann/arduino-audio-tools
ESP32-A2DP library
https://github.com/pschatzmann/ESP32-A2DP
and
https://github.com/pschatzmann/arduino-libhelix
Accelerometer
https://github.com/adafruit/Adafruit_BNO055

Acclerometer support library
https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR

Uses example code from
https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-basic-api/base-player-a2dp/base-player-a2dp.ino
https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-sdfat-a2dp/player-sdfat-a2dp.ino
https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-sd-i2s/player-sd-i2s.ino

What this does:
esp32-player-sd-a2dp

Audio file processing needed:
Open sound file in Audacity. Make sure that it contains 2 channels
- Select Tracks -> Resample and select 44100
- Export -> Export Audio -> Header Raw ; Signed 16 bit PCM

ESP32 WROOM Dev Board
Flash size: 4 MB

Pins
17 - SD NAND CS
18 - SPI Clock
19 - SPI MISO
23 - SPI MOSI

I2C
22 - SCL
21 - SDA
 */

#include "AudioTools.h"
#include "BluetoothA2DPSource.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

double xPos = 0, yPos = 0, headingVel = 0;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10; //how often to read data from the board
uint16_t PRINT_DELAY_MS = 500; // how often to print the data
uint16_t printCount = 0; //counter to avoid printing every 10MS sample

//velocity = accel*dt (dt in seconds)
//position = 0.5*accel*dt^2
double ACCEL_VEL_TRANSITION =  (double)(BNO055_SAMPLERATE_DELAY_MS) / 1000.0;
double ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
double DEG_2_RAD = 0.01745329251; //trig functions require radians, BNO055 outputs degrees

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

const char* file_name = "/Rage.raw";
File sound_file;
BluetoothA2DPSource a2dp_source;

#define SPI_SCK       18
#define SPI_MISO      19
#define SPI_MOSI      23
#define NAND_SPI_CS   17

// I2C
#define I2CSDA        21
#define I2CSCL        22

float oldyaw = 0;
float oldpitch = 0;
float oldroll = 0;
int playing = 0;

#define range 5

long starttime = 0;
long totalmusic = 0;
uint32_t prev_ms = 0;
uint32_t stats_ms = 0;

const int frame_size_bytes = sizeof(int16_t) * 2;

int32_t get_sound_data(Channels* data, int32_t len) {

    int32_t result_len = 0;
    if ( playing )
    {
      // the data in the file must be in int16 with 2 channels 
      size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes );
      // result is in number of frames
      result_len = result_len_bytes / frame_size_bytes;
      // ESP_LOGD("get_sound_data", "%d -> %d",len );
      totalmusic += result_len_bytes;
    }

    return result_len;    
}

void setup() {
  Serial.begin(115200);

  delay(2000);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  Serial.setDebugOutput(true);
  Serial.println( " " );  
  Serial.println( " " );  
  Serial.println("AirGuitar by Frank Cohen, fcohen@starlingwatch.com 2023");
 
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI );

  Wire.begin( I2CSDA, I2CSCL );

  if ( ! SD.begin( NAND_SPI_CS ) )
  {
    Serial.println(F("SD card failed"));
  }
  else
  {
    Serial.println(F("SD card mounted"));
  }

  sound_file = SD.open(file_name);
  
  delay(100);

  if (!bno.begin())
  {
    Serial.print("No BNO055 detected");
    while (1);
  }

  sensors_event_t orientationData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  oldyaw = orientationData.orientation.x + range;

  a2dp_source.start("JBL Charge 4", get_sound_data);  
  
  starttime = millis();
  totalmusic = 0;
  prev_ms = millis();
  stats_ms = millis();

  Serial.println("Started");
}

long time1 = 0;

void timestamp( String me )
{
  Serial.print( me );
  Serial.print( ", " );
  Serial.println( millis() );
}

void loop() { 

  if (millis() > prev_ms + 250)
  {
    prev_ms = millis();

    sensors_event_t orientationData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);

    float yaw = orientationData.orientation.x + range;
    
    if ( ( yaw - range < oldyaw ) && ( yaw + range > oldyaw ) )
    {
      playing = 0;
      //Serial.println( "" );              
    }
    else            
    {
      playing = 1;
      prev_ms += 1300;
      Serial.println( "*" );
      oldyaw = yaw;
    }
      
  }

  if (millis() > stats_ms + 2000) {
      stats_ms = millis();
      float rate = totalmusic / ( ( millis() - starttime ) / 1000 );
      Serial.print( "rate = " );
      Serial.print( rate );
      Serial.print( ", totalmusic = " );
      Serial.print( totalmusic );
      Serial.print( ", time = " );
      Serial.println( ( millis() - starttime ) / 1000 );
  }
        
}
