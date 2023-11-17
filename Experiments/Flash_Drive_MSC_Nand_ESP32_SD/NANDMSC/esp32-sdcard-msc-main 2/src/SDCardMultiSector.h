#pragma once

#include "SDCardIdf.h"

class SDCardMultiSector: public SDCardIdf
{
protected:
public:
  SDCardMultiSector(Stream &debug, const char *mount_point, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs);
  SDCardMultiSector(Stream &debug, const char *mount_point, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);
  bool writeSectors(uint8_t *src, size_t start_sector, size_t sector_count);
  bool readSectors(uint8_t *dst, size_t start_sector, size_t sector_count);
};