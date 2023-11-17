#include <Arduino.h>
#include "USB.h"
#include "USBMSC.h"

#include "SDCardArduino.h"
#include "SDCardMultiSector.h"
#include "SDCardLazyWrite.h"

#define BOOT_BUTTON 0

// #define SD_CARD_SPEED_TEST
#define SPEED_TEST_BUFFER_SIZE 4096
#define SPEED_TEST_NUMBER_SECTORS (SPEED_TEST_BUFFER_SIZE / 512)

#ifndef SD_CARD_SPEED_TEST
USBMSC msc;
USBCDC Serial;
#endif
SDCard *card;

void log(const char *str)
{
  Serial.println(str);
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
  // Serial.printf("Writing %d bytes to %d at offset\n", bufsize, lba, offset);
  // this writes a complete sector so we should return sector size on success
  if (card->writeSectors(buffer, lba, bufsize / card->getSectorSize()))
  {
    return bufsize;
  }
  return bufsize;
  // return -1;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  // Serial.printf("Reading %d bytes from %d at offset %d\n", bufsize, lba, offset);
  // this reads a complete sector so we should return sector size on success
  if (card->readSectors((uint8_t *)buffer, lba, bufsize / card->getSectorSize()))
  {
    return bufsize;
  }
  return -1;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject)
{
  Serial.printf("StartStop: %d %d %d\n", power_condition, start, load_eject);
  if (load_eject)
  {
#ifndef SD_CARD_SPEED_TEST
    msc.end();
#endif
  }
  return true;
}

bool isBootButtonClicked()
{
  return digitalRead(BOOT_BUTTON) == LOW;
}

void setup()
{
  pinMode(GPIO_NUM_2, OUTPUT);

#ifdef USE_SDIO
  card = new SDCardMultiSector(Serial, "/sd", SD_CARD_CLK, SD_CARD_CMD, SD_CARD_DAT0, SD_CARD_DAT1, SD_CARD_DAT2, SD_CARD_DAT3);
#else
  card = new SDCardLazyWrite(Serial, "/sd", SD_CARD_MISO, SD_CARD_MOSI, SD_CARD_CLK, SD_CARD_CS);
#endif

#ifdef SD_CARD_SPEED_TEST
#warning "SD_CARD_SPEED_TEST is enabled - this will potentially corrupt your SD Card!"
  Serial.begin(115200);
  // wait a bit of time so we can connect the serial monitor
  for (int i = 0; i < 10; i++)
  {
    Serial.printf("Waiting %d\n", i);
    delay(1000);
  }
  card->printCardInfo();
  Serial.printf("Sector Size=%d\n", card->getSectorSize());
  // allocate a buffer of SPEED_TEST_BUFFER_SIZE bytes and fill it with random numbers
  uint8_t *buffer = (uint8_t *)malloc(SPEED_TEST_BUFFER_SIZE);
  for (int i = 0; i < SPEED_TEST_BUFFER_SIZE; i++)
  {
    buffer[i] = random(0, 255);
  }
  Serial.printf("Starting test: %d, %d\n", SPEED_TEST_BUFFER_SIZE, SPEED_TEST_NUMBER_SECTORS);
  // write 400MBytes of data to the SD Card using the writeSectors method
  int total_write_bytes = 0;
  uint32_t start = millis();
  for (int times = 0; times < 100; times++)
  {
    for (int i = 0; i < 1024 * 1024 / SPEED_TEST_BUFFER_SIZE; i++)
    {
      if (card->writeSectors(buffer, 100 + i * SPEED_TEST_NUMBER_SECTORS, SPEED_TEST_NUMBER_SECTORS))
      {
        total_write_bytes += SPEED_TEST_BUFFER_SIZE;
        // Serial.printf("#");
      }
      else
      {
        Serial.printf("X");
      }
    }
    Serial.printf(".");
  }
  Serial.println();
  uint32_t end = millis();
  Serial.printf("Write %dBytes took %dms\n", total_write_bytes, end - start);
  // read 400MBytes of data from the SD Card using the readSectors method
  int total_read_bytes = 0;
  uint8_t *read_buffer = (uint8_t *)malloc(SPEED_TEST_BUFFER_SIZE);
  start = millis();
  for (int times = 0; times < 100; times++)
  {
    for (int i = 0; i < 1024 * 1024 / SPEED_TEST_BUFFER_SIZE; i++)
    {
      if (card->readSectors(read_buffer, 100 + i * SPEED_TEST_NUMBER_SECTORS, SPEED_TEST_NUMBER_SECTORS))
      {
        total_read_bytes += SPEED_TEST_BUFFER_SIZE;
        // check the numbers match
        // for(int j = 0; j<4096; j++) {
        //     if (read_buffer[j] != buffer[j]) {
        //         Serial.printf("Mismatch at %d\n", j);
        //         break;
        //     }
        // }
        // Serial.printf(".");
      }
      else
      {
        Serial.printf("X");
      }
    }
    Serial.printf(".");
  }
  Serial.println();
  end = millis();
  Serial.printf("Read %dBytes took %dms\n", total_read_bytes, end - start);
#else
  // keyboard.begin();
  msc.vendorID("ESP32");
  msc.productID("USB_MSC");
  msc.productRevision("1.0");
  msc.onRead(onRead);
  msc.onWrite(onWrite);
  msc.onStartStop(onStartStop);
  msc.mediaPresent(true);

  msc.begin(card->getSectorCount(), card->getSectorSize());
  Serial.begin(115200);
  USB.begin();
#endif
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(200);
  // if (isBootButtonClicked())
  // {
  //     if (MySD.cardType() == CARD_NONE)
  //     {
  //         log("No SD card");
  //     }
  // }
}