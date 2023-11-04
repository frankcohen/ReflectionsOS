/*

ESP32-S3 SPI Nand/SD SdFat Format for Fat16 recipe

by Frank Cohen, November 2, 2023, part of the Reflections open-source project

SdFat is an amazing open-source library for Arduino and ESP32 projects to use SD and Nand 
storage. It's been around since 2009. @greiman does an amazing job, even if SdFat is 
impossible to maintain. This is my contribution, a recipe for fellow ESP32 enthusiasts 
needing to format an SD for Fat16 over SPI. I wrote it for my 
[Reflections](https://github.com/frankcohen/ReflectionsOS) open-source project. 
It uses an ESP32-S3 and Nand/SD.

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

and this file is found at: https://github.com/frankcohen/ReflectionsOS/experiments/SdFat_ESP32_SPI_SD_NAND_Formatting

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

*/

#include "SPI.h"
#include "SdFat.h"
#include "sdios.h"

const int8_t DISABLE_CS_PIN = 12;

#define SD_CS_PIN 15

#define SPI_MOSI      35
#define SPI_MISO      37
#define SPI_SCK       36

// Display
#define Display_SPI_DC    5
#define Display_SPI_CS    12
#define Display_SPI_RST   0
#define Display_SPI_BK    6

SdFs sd;
cid_t cid;
csd_t csd;
scr_t scr;
uint8_t cmd6Data[64];
uint32_t eraseSize;
uint32_t ocr;
static ArduinoOutStream cout(Serial);

// #if SPI_DRIVER_SELECT == 3  // Must be set in SdFat/SdFatConfig.h

class MySpiClass : public SdSpiBaseClass {
 public:
  // Activate SPI hardware with correct speed and mode.
  void activate() { SPI.beginTransaction(m_spiSettings); }
  // Initialize the SPI bus.
  void begin(SdSpiConfig config) {
    (void)config;
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);
  }
  // Deactivate SPI hardware.
  void deactivate() { SPI.endTransaction(); }
  // Receive a byte.
  uint8_t receive() { return SPI.transfer(0XFF); }
  // Receive multiple bytes.
  // Replace this function if your board has multiple byte receive.
  uint8_t receive(uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      buf[i] = SPI.transfer(0XFF);
    }
    return 0;
  }
  // Send a byte.
  void send(uint8_t data) { SPI.transfer(data); }
  // Send multiple bytes.
  // Replace this function if your board has multiple byte send.
  void send(const uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      SPI.transfer(buf[i]);
    }
  }
  // Save SPISettings for new max SCK frequency
  void setSckSpeed(uint32_t maxSck) {
    m_spiSettings = SPISettings(maxSck, MSBFIRST, SPI_MODE0);
  }

 private:
  SPISettings m_spiSettings;
} mySpi;
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50), &mySpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &mySpi)
#endif  // ENABLE_DEDICATED_SPI

void setup() {
  Serial.begin(115200);
  long time = millis();
  while (!Serial && (millis() < time + 5000))
    ;  // wait up to 5 seconds for Arduino Serial Monitor
  Serial.setDebugOutput(true);

  Serial.println("Starting");

  pinMode(SD_CS_PIN, OUTPUT );
  digitalWrite(SD_CS_PIN, LOW);

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, LOW);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);

  delay(1000);

  cout << F("SdFat version: ") << SD_FAT_VERSION_STR << endl;
  printConfig(SD_CONFIG);

  Serial.println("Done");
}

//------------------------------------------------------------------------------
void cidDmp() {
  cout << F("\nManufacturer ID: ");
  cout << uppercase << showbase << hex << int(cid.mid) << dec << endl;
  cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++) {
    cout << cid.pnm[i];
  }
  cout << F("\nRevision: ") << cid.prvN() << '.' << cid.prvM() << endl;
  cout << F("Serial number: ") << hex << cid.psn() << dec << endl;
  cout << F("Manufacturing date: ");
  cout << cid.mdtMonth() << '/' << cid.mdtYear() << endl;
  cout << endl;
}
//------------------------------------------------------------------------------
void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}
//------------------------------------------------------------------------------
void csdDmp() {
  eraseSize = csd.eraseSize();
  cout << F("cardSize: ") << 0.000512 * csd.capacity();
  cout << F(" MB (MB = 1,000,000 bytes)\n");

  cout << F("flashEraseSize: ") << int(eraseSize) << F(" blocks\n");
  cout << F("eraseSingleBlock: ");
  if (csd.eraseSingleBlock()) {
    cout << F("true\n");
  } else {
    cout << F("false\n");
  }
  cout << F("dataAfterErase: ");
  if (scr.dataAfterErase()) {
    cout << F("ones\n");
  } else {
    cout << F("zeros\n");
  }
}
//------------------------------------------------------------------------------
void errorPrint() {
  if (sd.sdErrorCode()) {
    cout << F("SD errorCode: ") << hex << showbase;
    printSdErrorSymbol(&Serial, sd.sdErrorCode());
    cout << F(" = ") << int(sd.sdErrorCode()) << endl;
    cout << F("SD errorData = ") << int(sd.sdErrorData()) << dec << endl;
  }
}
//------------------------------------------------------------------------------
bool mbrDmp() {
  MbrSector_t mbr;
  bool valid = true;
  if (!sd.card()->readSector(0, (uint8_t *)&mbr)) {
    cout << F("\nread MBR failed.\n");
    errorPrint();
    return false;
  }
  cout << F("\nSD Partition Table\n");
  cout << F("part,boot,bgnCHS[3],type,endCHS[3],start,length\n");
  for (uint8_t ip = 1; ip < 5; ip++) {
    MbrPart_t *pt = &mbr.part[ip - 1];
    if ((pt->boot != 0 && pt->boot != 0X80) ||
        getLe32(pt->relativeSectors) > csd.capacity()) {
      valid = false;
    }
    cout << int(ip) << ',' << uppercase << showbase << hex;
    cout << int(pt->boot) << ',';
    for (int i = 0; i < 3; i++) {
      cout << int(pt->beginCHS[i]) << ',';
    }
    cout << int(pt->type) << ',';
    for (int i = 0; i < 3; i++) {
      cout << int(pt->endCHS[i]) << ',';
    }
    cout << dec << getLe32(pt->relativeSectors) << ',';
    cout << getLe32(pt->totalSectors) << endl;
  }
  if (!valid) {
    cout << F("\nMBR not valid, assuming Super Floppy format.\n");
  }
  return true;
}
//------------------------------------------------------------------------------
void dmpVol() {
  cout << F("\nScanning FAT, please wait.\n");
  int32_t freeClusterCount = sd.freeClusterCount();
  if (sd.fatType() <= 32) {
    cout << F("\nVolume is FAT") << int(sd.fatType()) << endl;
  } else {
    cout << F("\nVolume is exFAT\n");
  }
  cout << F("sectorsPerCluster: ") << sd.sectorsPerCluster() << endl;
  cout << F("fatStartSector:    ") << sd.fatStartSector() << endl;
  cout << F("dataStartSector:   ") << sd.dataStartSector() << endl;
  cout << F("clusterCount:      ") << sd.clusterCount() << endl;
  cout << F("freeClusterCount:  ");
  if (freeClusterCount >= 0) {
    cout << freeClusterCount << endl;
  } else {
    cout << F("failed\n");
    errorPrint();
  }
}
//------------------------------------------------------------------------------
void printCardType() {
  cout << F("\nCard type: ");

  switch (sd.card()->type()) {
    case SD_CARD_TYPE_SD1:
      cout << F("SD1\n");
      break;

    case SD_CARD_TYPE_SD2:
      cout << F("SD2\n");
      break;

    case SD_CARD_TYPE_SDHC:
      if (csd.capacity() < 70000000) {
        cout << F("SDHC\n");
      } else {
        cout << F("SDXC\n");
      }
      break;

    default:
      cout << F("Unknown\n");
  }
}
//------------------------------------------------------------------------------
void printConfig(SdSpiConfig config) {
  if (DISABLE_CS_PIN < 0) {
    cout << F(
        "\nAssuming the SD is the only SPI device.\n"
        "Edit DISABLE_CS_PIN to disable an SPI device.\n");
  } else {
    cout << F("\nDisabling SPI device on pin ");
    cout << int(DISABLE_CS_PIN) << endl;
    pinMode(DISABLE_CS_PIN, OUTPUT);
    digitalWrite(DISABLE_CS_PIN, HIGH);
  }
  cout << F("\nAssuming the SD chip select pin is: ") << int(config.csPin);
  cout << F("\nEdit SD_CS_PIN to change the SD chip select pin.\n");
}
//------------------------------------------------------------------------------
void printConfig(SdioConfig config) {
  (void)config;
  cout << F("Assuming an SDIO interface.\n");
}

//------------------------------------------------------------------------------
void loop() {
  // Read any existing Serial data.
  clearSerialInput();

  // F stores strings in flash to save RAM
  cout << F("\ntype any character to start\n");
  while (!Serial.available()) {
    yield();
  }
  uint32_t t = millis();
  if (!sd.cardBegin(SD_CONFIG)) {
    cout << F(
        "\nSD initialization failed.\n"
        "Do not reformat the card!\n"
        "Is the card correctly inserted?\n"
        "Is there a wiring/soldering problem?\n");
    if (isSpi(SD_CONFIG)) {
      cout << F(
          "Is SD_CS_PIN set to the correct value?\n"
          "Does another SPI device need to be disabled?\n");
    }
    errorPrint();
    return;
  }
  t = millis() - t;
  cout << F("init time: ") << dec << t << " ms" << endl;

  if (!sd.card()->readCID(&cid) || !sd.card()->readCSD(&csd) ||
      !sd.card()->readOCR(&ocr) || !sd.card()->readSCR(&scr)) {
    cout << F("readInfo failed\n");
    errorPrint();
    return;
  }
  printCardType();
  cout << F("sdSpecVer: ") << 0.01 * scr.sdSpecVer() << endl;
  cout << F("HighSpeedMode: ");
  if (scr.sdSpecVer() && sd.card()->cardCMD6(0X00FFFFFF, cmd6Data) &&
      (2 & cmd6Data[13])) {
    cout << F("true\n");
  } else {
    cout << F("false\n");
  }
  cidDmp();
  csdDmp();
  cout << F("\nOCR: ") << uppercase << showbase;
  cout << hex << ocr << dec << endl;
  if (!mbrDmp()) {
    return;
  }
  if (!sd.volumeBegin()) {
    cout << F("\nvolumeBegin failed. Is the card formatted?\n");
    errorPrint();
    return;
  }
  dmpVol();
}

