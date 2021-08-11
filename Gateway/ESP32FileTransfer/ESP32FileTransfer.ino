#define ESP_SPP_MAX_MTU 4096

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <TJpg_Decoder.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define SD_CS 5

BluetoothSerial SerialBT;

TFT_eSPI tft = TFT_eSPI();

#define FPS 15
#define MJPEG_FILENAME "/sample_folder/240_15fps.mjpeg"

#define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 4)
#define READ_BUFFER_SIZE 2048

BLECharacteristic *pTxCharacteristic;
unsigned int uptime = 0;
bool deviceConnected = false;
bool classicConnected = false;
bool congest = false;
bool _file_received = false;
uint16_t myhandle;

char buf[30];
uint8_t lbuf[300];
String tmp;

double byte_cnt = 0;
double tol_len = 0;
const int send_len = 250;
unsigned int packet_cnt = 0;
int pre_percent = 0;
File file;

bool rcv_data = false;
bool begin_transfer = false;
String filename = "";

void open_file(const char * filename);
void close_file();
void send_cmd(String cmd);
void initBT();
void extract_files();

int next_frame = 0;
int skipped_frames = 0;
unsigned long total_read_video = 0;
unsigned long total_play_video = 0;
unsigned long total_remain = 0;
unsigned long start_ms, curr_ms, next_frame_ms;

uint8_t *_read_buf;
uint8_t *_mjpeg_buf;
int32_t _mjpeg_buf_offset = 0;

int32_t _inputindex = 0;
int32_t _buf_read;
int32_t _remain = 0;
uint32_t _fileindex;

File _input;

#define DEST_FS_USES_SD
#define ARDUINOJSON_DEFAULT_NESTING_LIMIT 10
#include <ESP32-targz.h>
#include <ArduinoJson.h>

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
          tmp = "start_download";
          tmp.toCharArray(buf, tmp.length()+1);
          pTxCharacteristic->setValue(buf);
          pTxCharacteristic->notify();
        }
        else if(rxMsg.indexOf("fn:") >= 0){
          //Serial.println(rxMsg);
          int fli = rxMsg.indexOf("fl:");
          filename = '/' + rxMsg.substring(3,fli-1);
          Serial.println(filename);
          byte_cnt = rxMsg.substring(fli+3).toInt();
          Serial.println(byte_cnt);
        }
        else if(rxMsg == "finish"){
          if(tol_len == byte_cnt){
            rcv_data = false;
            Serial.println("FILE RECEIVED COMPLETELY");
            tmp = "completed";
            tmp.toCharArray(buf, tmp.length()+1);
            pTxCharacteristic->setValue(buf);
            pTxCharacteristic->notify();
            close_file();
            _file_received = true;
          }
          else{
            rcv_data = false;
            Serial.println("FILE NOT RECEIVED COMPLETELY");
            tmp = "incompleted";
            tmp.toCharArray(buf, tmp.length()+1);
            pTxCharacteristic->setValue(buf);
            pTxCharacteristic->notify();
            close_file();
            _file_received = false;
          }
        }
      }
    }
};

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  tft.begin();
  tft.setRotation(2);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(TFT_BLACK);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  
  show_text("Start");
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

  SerialBT.enableSSP();
  SerialBT.register_callback(btCallback);
  Serial.println("Device Started... Now Pair with ESP32");
}

void deinitBT(){
  BLEDevice::deinit(true);
  SerialBT.end();
}

void initSD(){
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  else{
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      return;
    }
    else{
      Serial.println("Initialized SD card...");
      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
      initBT();
    }
  }
}

void open_file(const char * filename){
  file = SD.open(filename, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    SerialBT.println("Failed");
  }
  else{
    Serial.println("File Opened");
    show_text("File Opened " + String(filename));
    SerialBT.println("File Opened");
    tol_len = 0;
  }
}

void close_file(){
  file.close();
  Serial.println("File Closed");
  show_text("File Closed");
  tol_len = 0;
}

void CustomProgressCallback( uint8_t progress ){
  if(pre_percent != progress){
    pre_percent = progress;
    Serial.print("Extracted : ");
    Serial.print(progress);
    Serial.println(" %");
    show_text("Extracted : " + String(progress) + " %");
  }
}

void CustomTarStatusProgressCallback( const char* name, size_t size, size_t total_unpacked ){
  Serial.printf("[TAR] %-32s %8d bytes - %8d Total bytes\n", name, size, total_unpacked );
  show_text("Extracting : " + String(name));
  delay(500);
}

void extract_files(){
  show_text("Extracting TAR");
  Serial.println("Extracting TAR");
  delay(1000);
  pre_percent = 0;
  const char* tarFile = "/sample.tar";
  const char* tarFolder = "/sample_folder";
  TarUnpacker *TARUnpacker = new TarUnpacker();
  TARUnpacker->setTarVerify( true );
  TARUnpacker->haltOnError( true );
  TARUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
  TARUnpacker->setTarProgressCallback( /*BaseUnpacker::defaultProgressCallback*/ CustomProgressCallback); // prints the untarring progress for each individual file
  TARUnpacker->setTarStatusProgressCallback( /*BaseUnpacker::defaultTarStatusProgressCallback*/ CustomTarStatusProgressCallback ); // print the filenames as they're expanded
  TARUnpacker->setTarMessageCallback( BaseUnpacker::targzPrintLoggerCallback); // tar log verbosity

  if( !TARUnpacker->tarExpander(SD, tarFile, SD, tarFolder ) ) {
    Serial.print("tarExpander failed with return code ");
    Serial.println(TARUnpacker->tarGzGetError());
  }
  Serial.println("Extracting Complete");
  show_text("Extracting Complete");
  delay(1000);
  parse_json();
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap){
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void parse_json(){
  File json_file = SD.open("/sample_folder/manifest.json");
  if (!json_file || json_file.isDirectory())
  {
    Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
  }
  else
  {
    DynamicJsonDocument jsonBuffer(2048);
    String json_string = "";
    int bcnt = 0;
    Serial.println("Parsing Json File ==>");
    while (json_file.available()) {
      char c=json_file.read();
      if(c == '{'){
        bcnt++;
      }
      else if(c == '}'){
        bcnt--;
        if(bcnt == 0){
          json_string += c;
          break;
        }
      }
      else if(c == '"'){
        json_string += '\"';
      }
      if(!isWhitespace(c) && !isSpace(c) && c != '\n' && c != '"') json_string += c;
      //json_string += c;
    }
    json_file.close();
    Serial.println(json_string);
    
    DeserializationError error = deserializeJson(jsonBuffer, json_string);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    Serial.println("Parsing Completed");
    Serial.print("Title : ");
    Serial.println((const char*)jsonBuffer["ReflectionsShow"]["title"]);
    Serial.print("Showname : ");
    Serial.println((const char*)jsonBuffer["ReflectionsShow"]["showname"]);
    Serial.print("Event Name : ");
    Serial.println((const char*)jsonBuffer["ReflectionsShow"]["events"]["event"]["Name"]);
    show_text((const char*)jsonBuffer["ReflectionsShow"]["title"]);
    delay(2000);
    play_video();
  }
}

void delete_folder(){
  show_text("Deleting All files");
  Serial.println("Removing Sample Folder");
  for(int j=2;j>0;j--) {
    Serial.print("Deleting in ");
    Serial.print(j);
    Serial.println(" Seconds");
    delay(1000);
  }
  File root = SD.open("/sample_folder");
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      Serial.print("Deleting FILE: ");
      Serial.print(file.name());
      if(SD.remove(file.name())){
        Serial.println(" => Deleted Successfully");
      }
      else{
        Serial.println(" => Deleting Failed");
      }
    }
    file = root.openNextFile();
  }
  
  if(SD.rmdir("/sample_folder")){
    Serial.println("Removed Sample Folder Successfully");
  }
  else{
    Serial.println("Removing Sample Folder Failed");
  }
  delay(1000);
  if(SD.remove("/sample.tar")){
    Serial.println("Removed Sample Tar Successfully");
  }
  else{
    Serial.println("Removing Sample Tar Failed");
  }
  Serial.println();
  show_text("Deleting Complete");
}

void delete_tar(){
  show_text("Deleting Tar");
  if(SD.remove("/sample.tar")){
    Serial.println("Removed Sample Tar Successfully");
  }
  else{
    Serial.println("Removing Sample Tar Failed");
  }
  show_text("Deleting Complete");
}

void play_video() {
  show_text("Playing Video");
  delay(2000);
  _input = SD.open(MJPEG_FILENAME);
  if (!_input || _input.isDirectory())
  {
    Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
  }
  else
  {
    _mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
    _read_buf = (uint8_t *)malloc(READ_BUFFER_SIZE);
    if (!_mjpeg_buf || !_read_buf)
    {
      Serial.println(F("mjpeg_buf malloc failed!"));
    }
    else
    {
      Serial.println(F("MJPEG video start"));
      while (_input.available()) {
        readMjpegBuf();
        TJpgDec.drawJpg(0, 40, (const uint8_t * )_mjpeg_buf, _mjpeg_buf_offset); 
      }
      _input.close();
      Serial.println(F("MJPEG video end"));
      show_text("Playing Complete");
      delay(2000);
      delete_folder();
    }
  }
}

bool readMjpegBuf(){
  if (_inputindex == 0)
  {
    _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
    _inputindex += _buf_read;
  }

  _mjpeg_buf_offset = 0;
    int i = 3;
    bool found_FFD9 = false;
    if (_buf_read > 0)
    {
      i = 3;
      while ((_buf_read > 0) && (!found_FFD9))
      {
        if ((_mjpeg_buf_offset > 0) && (_mjpeg_buf[_mjpeg_buf_offset - 1] == 0xFF) && (_read_buf[0] == 0xD9)) // JPEG trailer
        {
          found_FFD9 = true;
        }
        else
        {
          while ((i < _buf_read) && (!found_FFD9))
          {
            if ((_read_buf[i] == 0xFF) && (_read_buf[i + 1] == 0xD9)) // JPEG trailer
            {
              found_FFD9 = true;
              ++i;
            }
            ++i;
          }
        }

        // Serial.printf("i: %d\n", i);
        memcpy(_mjpeg_buf + _mjpeg_buf_offset, _read_buf, i);
        _mjpeg_buf_offset += i;
        size_t o = _buf_read - i;
        if (o > 0)
        {
          // Serial.printf("o: %d\n", o);
          memcpy(_read_buf, _read_buf + i, o);
          _buf_read = _input.read(_read_buf + o, READ_BUFFER_SIZE - o);
          _inputindex += _buf_read;
          _buf_read += o;
          // Serial.printf("_buf_read: %d\n", _buf_read);
        }
        else
        {
          _buf_read = _input.read(_read_buf, READ_BUFFER_SIZE);
          _inputindex += _buf_read;
        }
        i = 0;
      }
      if (found_FFD9)
      {
        return true;
      }
    }

    return false;
}

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    show_text("Client Connected");
    myhandle = param->open.handle;
    classicConnected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT){
    Serial.println("Client disconnected");
    show_text("Client disconnected");
    classicConnected = false;
    if(rcv_data){
      Serial.println("File Not Received Completely");
      show_text("File Not Received Completely");
      delete_tar();
    }
  }
  else if(event == ESP_SPP_DATA_IND_EVT){
    int datalen = param->data_ind.len;
    tol_len = tol_len + datalen;
    float percent = float(tol_len/byte_cnt) * 100;
    if(pre_percent != int(percent)){
      pre_percent = int(percent);
      Serial.print("Received : ");
      Serial.print(percent);
      Serial.print(" %  => ");
      Serial.println(datalen);
      show_text("Received : " + String(percent) + " %");
    }
    file.write(param->data_ind.data, datalen);
  }
  else if(event == ESP_SPP_CONG_EVT){
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

void show_text(String msg){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  tft.println(msg);
}

void loop() {
  delay(2000);
  if(_file_received){
    Serial.println("File Received Completely");
    show_text("File Received Completely");
    //deinitBT();
    extract_files();
    _file_received = false;
    //ESP.restart();
  }
}
