/*
 * Send image to ESP32 Bluetooth through serial and store it as text file
 *
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * February 14, 2021 Happy Valentines Day
 *
 * Notes
 *
 * Please refer this android app, https://community.appinventor.mit.edu/t/bluetooth-hc06-send-receive-image-jpg-file-to-from-arduino-sdcard-reader/2150
 * While using this app from above link for testing, ensure editing your current ESP's mac address. 
 *
 *
 * Please refer this for sd card module connection to ESP32, https://acoptex.com/project/1285/basics-project-072y-esp32-development-board-with-sd-card-module-at-acoptexcom/#sthash.Jx6nGfTR.ktwnfALS.dpbs
 * I have used HSPI pins for sd card module interface with ESP32
 *
 * I have used esp32-micro-sdcard-master library (from libraries folder).
*/

#include <mySD.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
// HSPI Pins
#define MOSI 13
#define MISO 12
#define SCK 14
#define CS 15
File miarchivo;
char rx_byte = 0;

void setup() {
  Serial.begin(115200);//initialise serial communication at 115200 bps
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
  Serial.print("Initializing SD card...");
  /* initialize SD library with SPI pins */
  if (!SD.begin(CS, MOSI, MISO, SCK)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  /* Begin at the root "/" */
//  root = SD.open("/");
//  if (root) {    
//    printDirectory(root, 0);
//    root.close();
//  } else {
//    Serial.println("error opening logging.txt");
//  }
    
    Serial.println("done!");
    miarchivo = SD.open("pozo_4.txt", FILE_WRITE); // Abre el archivo.
}

void loop() {
  if(SerialBT.available()) {
    rx_byte = SerialBT.read(); // Take the character.
         if (rx_byte != '*') {
          miarchivo.print(rx_byte); // Save character.
         } else {
          miarchivo.close(); // Close the file.
          Serial.println("Recorded.");
         }
  }
}
