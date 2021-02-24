#include <mySD.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

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
