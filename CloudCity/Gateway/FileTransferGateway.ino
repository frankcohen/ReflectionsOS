/*
 * Transfer files between Seuss display board and Android mobile device using Bluetooth classic
 * This is the ESP32 side of the Gateway, it runs on the Seuss Display Board
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

   Thanks to Manoj Mishra for coding the Bluetooth file transfer protocol on Android and ESP32.
   Contact Manoj at: Manoj Mishra, Indore, India, http://nevalosstechnologies.com/
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define SD_CS 5

BluetoothSerial SerialBT;

BLECharacteristic *pTxCharacteristic;
unsigned int uptime = 0;
bool deviceConnected = false;
bool classicConnected = false;
bool congest = false;
uint16_t myhandle;

char buf[30];
uint8_t lbuf[300];
String tmp;

double byte_cnt = 0;
double tol_len = 0;
const int send_len = 250;
unsigned int packet_cnt = 0;
File file;

bool rcv_data = false;
bool begin_transfer = false;
String filename = "";

void open_file(const char * filename);
void close_file();
void send_cmd(String cmd);
void initBT();
uint8_t CRC8( uint8_t *addr, uint8_t len);

class MySeverCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer *pServer){
    deviceConnected = true;
    Serial.println("Connected");
  }

  void onDisconnect(BLEServer *pServer){
    deviceConnected = false;
    Serial.println("DisConnected");
    BLEDevice::startAdvertising();
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0){
        String rxMsg = String(rxValue.c_str());
        if(rxMsg == "download"){
          delay(500);
          Serial.println("Downloading");
          rcv_data = true;
          open_file(filename.c_str());
        }
        else if(rxMsg.indexOf("fn:") >= 0){
          //Serial.println(rxMsg);
          int fli = rxMsg.indexOf("fl:");
          filename = '/' + rxMsg.substring(3,fli-1);
          Serial.println(filename);
          byte_cnt = rxMsg.substring(fli+3).toInt();
          Serial.println(byte_cnt);
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  initBT();
  initSD();
}

void initBT(){
  BLEDevice::init("ESP32");
  if(!SerialBT.begin("ESP32")){
    Serial.println("An Error occured...");
    ESP.restart();
    return;
  }
  else{
    Serial.println("Bluetooth Initialized");
  }
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MySeverCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Waiting For Client Connection");

  SerialBT.register_callback(btCallback);
  Serial.println("Device Started... Now Pair with ESP32");
}

void initSD(){
  if(!SD.begin( 4, SPI, 80000000  ))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initialized SD card...");
}

void open_file(const char * filename){
  file = SD.open(filename, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    SerialBT.println("Failed");
  }
  else{
    Serial.println("File Opened");
    SerialBT.println("File Opened");
    tol_len = 0;
  }
}

void close_file(){
  file.close();
  Serial.println("File Closed");
  tol_len = 0;
}

uint8_t CRC8( uint8_t *ar, uint8_t len)
{
   uint8_t crc = 0;
   for(uint8_t i =0 ; i < len; i++){
     crc ^= ar[i];
   }
   return crc;
}

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    myhandle = param->open.handle;
    classicConnected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT){
    Serial.println("Client disconnected");
    classicConnected = false;
  }
  else if(event == ESP_SPP_DATA_IND_EVT){
    int datalen = param->data_ind.len;
    tol_len = tol_len + datalen;
    Serial.print("Received : ");
    Serial.print(float(tol_len/byte_cnt) * 100);
    Serial.println(" %");
    file.write(param->data_ind.data, datalen);
    if(tol_len == byte_cnt){
      rcv_data = false;
      Serial.println("FILE RECEIVED COMPLETELY");
      tmp = "completed";
      tmp.toCharArray(buf, tmp.length()+1);
      pTxCharacteristic->setValue(buf);
      pTxCharacteristic->notify();
      close_file();
    }
    else if(tol_len > byte_cnt){
      rcv_data = false;
      Serial.println("ERROR");
      close_file();
    }
  }
  else if(event == ESP_SPP_CONG_EVT)
  {
    Serial.print("ESP_SPP_CONG_EVT cong = ");
    Serial.println( param->cong.cong );
    congest = param->cong.cong;
  }
}

void send_cmd(String cmd){
  tmp = cmd;
  tmp.toCharArray(buf, tmp.length()+1);
  pTxCharacteristic->setValue(buf);
  pTxCharacteristic->notify();
}

void loop() {
  if(Serial.available()){
    String inputString = "";
    while (Serial.available()) {
      char inChar = (char)Serial.read();
      inputString += inChar;
      if (inChar == '\n') {
        if(inputString.startsWith("send")){
          if(deviceConnected){
            Serial.println("Requesting File");
            send_cmd("send");
          }
          else{
            Serial.println("Phone Not Connected, Retry after connecting Client");
          }
        }
        else{
          Serial.print(inputString);
        }
      }
    }
  }
  else{
    delay(1000);
  }
}
