#include <Arduino.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include "SDCardMultiSector.h"

SDCardMultiSector::SDCardMultiSector(Stream &debug, const char *mount_point, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
: SDCardIdf(debug, mount_point, miso, mosi, clk, cs)
{
}

SDCardMultiSector::SDCardMultiSector(Stream &debug, const char *mount_point, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
: SDCardIdf(debug, mount_point, clk, cmd, d0, d1, d2, d3)
{
}

bool SDCardMultiSector::writeSectors(uint8_t *src, size_t start_sector, size_t sector_count) {
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  digitalWrite(GPIO_NUM_2, HIGH);
  esp_err_t res = sdmmc_write_sectors(m_card, src, start_sector, sector_count);
  digitalWrite(GPIO_NUM_2, LOW);
  xSemaphoreGive(m_mutex);
  return res == ESP_OK;
}

bool SDCardMultiSector::readSectors(uint8_t *dst, size_t start_sector, size_t sector_count) {
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  digitalWrite(GPIO_NUM_2, HIGH);
  esp_err_t res = sdmmc_read_sectors(m_card, dst, start_sector, sector_count);
  digitalWrite(GPIO_NUM_2, LOW);
  xSemaphoreGive(m_mutex);
  return res == ESP_OK;
}
