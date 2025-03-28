/*!
 * @file Adafruit_H3LIS331.cpp
 */

#include "Arduino.h"

#include <Adafruit_H3LIS331.h>
#include <Wire.h>

/*!
 *  @brief  Instantiates a new H3LIS331 class
 */
Adafruit_H3LIS331::Adafruit_H3LIS331(){};

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @param  sensor_id
 *            The user-defined ID to differentiate different sensors
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_H3LIS331::begin_I2C(uint8_t i2c_address, TwoWire *wire,
                                  int32_t sensor_id) {
  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  return _init(sensor_id);
}

/*!
 *    @brief  Sets up the hardware and initializes hardware SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  theSPI The SPI object to be used for SPI connections.
 *    @param  sensor_id
 *            The user-defined ID to differentiate different sensors
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_H3LIS331::begin_SPI(uint8_t cs_pin, SPIClass *theSPI,
                                  int32_t sensor_id) {
  i2c_dev = NULL;

  if (spi_dev) {
    delete spi_dev; // remove old interface
  }
  spi_dev = new Adafruit_SPIDevice(cs_pin,
                                   1000000,               // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0,             // data mode
                                   theSPI);
  if (!spi_dev->begin()) {
    return false;
  }

  return _init(sensor_id);
}

/*!
 *    @brief  Sets up the hardware and initializes software SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  sck_pin The arduino pin # connected to SPI clock
 *    @param  miso_pin The arduino pin # connected to SPI MISO
 *    @param  mosi_pin The arduino pin # connected to SPI MOSI
 *    @param  sensor_id
 *            The user-defined ID to differentiate different sensors
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_H3LIS331::begin_SPI(int8_t cs_pin, int8_t sck_pin,
                                  int8_t miso_pin, int8_t mosi_pin,
                                  int32_t sensor_id) {
  i2c_dev = NULL;

  if (spi_dev) {
    delete spi_dev; // remove old interface
  }
  spi_dev = new Adafruit_SPIDevice(cs_pin, sck_pin, miso_pin, mosi_pin,
                                   1000000,               // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0);            // data mode
  if (!spi_dev->begin()) {
    return false;
  }

  return _init(sensor_id);
}

#define LIS331_REG_WHOAMI                                                      \
  0x0F /**< Device identification register. [0, 0, 1, 1, 0, 0, 1, 1] */


/*!  @brief Initializer for post i2c/spi init
 *   @param sensor_id Optional unique ID for the sensor set
 *   @returns True if chip identified and initialized
 */
bool Adafruit_H3LIS331::_init(int32_t sensor_id) {

  uint8_t device_id = getDeviceID();
  /* Check connection */
  if (device_id != LIS331_CHIP_ID) {
    /* No H3LIS331 detected ... return false */
    // Serial.print("Chip ID: 0x");Serial.println(device_id, HEX);
    return false;
  }
  Adafruit_BusIO_Register _ctrl1 = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LIS331_REG_CTRL1, 1);
  _ctrl1.write(0x07); // enable all axes, normal mode
  enableHighPassFilter(false);
  setDataRate(LIS331_DATARATE_1000_HZ);
  setRange(H3LIS331_RANGE_400_G);

  return true;
}

void Adafruit_H3LIS331::_scaleValues(void) {

  // actually 12 bit but left justified
  x >>= 4;
  y >>= 4;
  z >>= 4;
  uint8_t range = getRange();
  uint16_t scale_max = 1;
  if (range == H3LIS331_RANGE_100_G)
    scale_max = 100;
  if (range == H3LIS331_RANGE_200_G)
    scale_max = 200;
  if (range == H3LIS331_RANGE_400_G)
    scale_max = 400;

  float lsb_value = 2 * scale_max * (float)1 / 4096;

  x_g = ((float)x * lsb_value);
  y_g = ((float)y * lsb_value);
  z_g = ((float)z * lsb_value);
}

/**
 *  @brief  Sets the measurement range for the H3LIS331
 *  @param  range The range to set
 */

void Adafruit_H3LIS331::setRange(h3lis331dl_range_t range) {
  writeRange((uint8_t)range);
}

/**
 *   @brief  Gets the measurement range for the H3LIS331
 *   @return The range value
 */
h3lis331dl_range_t Adafruit_H3LIS331::getRange(void) {

  return (h3lis331dl_range_t)readRange();
}
