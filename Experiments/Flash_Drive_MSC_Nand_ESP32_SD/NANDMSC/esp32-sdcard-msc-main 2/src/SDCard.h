#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/sdmmc_types.h>
#include <driver/sdspi_host.h>

#include <string>

class Stream;
class SDCard
{
protected:
  std::string m_mount_point;
  int m_sector_size = 0;
  int m_sector_count = 0;
  Stream &m_debug;
public:
  SDCard(Stream &debug, const char *mount_point);
  virtual ~SDCard();
  virtual bool writeSectors(uint8_t *src, size_t start_sector, size_t sector_count) = 0;
  virtual bool readSectors(uint8_t *dst, size_t start_sector, size_t sector_count) = 0;
  virtual void printCardInfo() = 0;
  size_t getSectorSize() { return m_sector_size; }
  size_t getSectorCount() { return m_sector_count; }
  const std::string &get_mount_point() { return m_mount_point; }
};
