#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "SDCardArduino.h"

SDCardArduino::SDCardArduino(Stream &debug, const char *mount_point, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
    : SDCard(debug, mount_point)
{
  static SPIClass spi(HSPI);
  spi.begin(clk, miso, mosi, cs);
  if (SD.begin(cs, spi, 80000000, mount_point))
  {
    debug.println("SD card initialized");
  }
  else
  {
    debug.println("SD card initialization failed");
  }
  m_sector_size = SD.sectorSize();
  m_sector_count = SD.numSectors();
}

SDCardArduino::~SDCardArduino()
{
  SD.end();
}

void SDCardArduino::printCardInfo()
{
  m_debug.printf("Card type: %d\n", SD.cardType());
  if (SD.cardType() == CARD_NONE)
  {
    m_debug.println("No SD card attached");
    return;
  }
  m_debug.printf("Card size: %lluMB\n", SD.cardSize() / (1024 * 1024));
}

bool SDCardArduino::writeSectors(uint8_t *src, size_t start_sector, size_t sector_count)
{
  digitalWrite(GPIO_NUM_2, HIGH);
  bool res = true;
  for (int i = 0; i < sector_count; i++)
  {
    res = SD.writeRAW((uint8_t *)src, start_sector + i);
    if (!res)
    {
      break;
    }
    src += m_sector_size;
  }
  digitalWrite(GPIO_NUM_2, LOW);
  return res;
}

bool SDCardArduino::readSectors(uint8_t *dst, size_t start_sector, size_t sector_count)
{
  digitalWrite(GPIO_NUM_2, HIGH);
  bool res = true;
  for (int i = 0; i < sector_count; i++)
  {
    res = SD.readRAW((uint8_t *)dst, start_sector + i);
    if (!res)
    {
      break;
    }
    dst += m_sector_size;
  }
  digitalWrite(GPIO_NUM_2, LOW);
  return res;
}
