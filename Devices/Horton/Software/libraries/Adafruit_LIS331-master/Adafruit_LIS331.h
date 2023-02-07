/*!
 *  @file Adafruit_LIS331.h
 *
 *  @mainpage Adafruit LIS331 breakout board
 *
 *  @section intro_sec Introduction
 * *  This is a library for the Adafruit LIS331X Family of Accelerometer
 breakout boards
 *
 *  Designed specifically to work with:
 *  * [Adafruit LIS331HH Triple-Axis Accelerometer
 (+-6g/12g/24g)](https://www.adafruit.com/product/4XXX)
 *  * [Adafruit H3LIS331 High-G Triple-Axis Accelerometer
 (+-100g/200g/400g)](https://www.adafruit.com/product/4XXX)

 *  Pick one up today in the adafruit shop!
 *
 *	These sensors communicate over I2C or SPI (our library code supports
 *both) so you can share it with a bunch of other sensors on the same I2C bus.
 *  There's an address selection pin so you can have two accelerometers share an
 *I2C bus.
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  @section author Author
 *
 *  Bryan Siepert / K. Townsend / Limor Fried (Adafruit Industries)
 *
 *  @section license License
 *
 *  BSD license, all text above must be included in any redistribution
 */

#ifndef ADAFRUIT_LIS331_H
#define ADAFRUIT_LIS331_H

#include "Arduino.h"

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_Sensor.h>
/** Default I2C ADDRESS. If SDO/SA0 is 3V, its 0x19**/
#define LIS331_DEFAULT_ADDRESS (0x18)
#define LIS331_CHIP_ID                                                         \
  0x32 ///< The default response to WHO_AM_I for the H3LIS331 and LIS331HH

#define LIS331_REG_WHOAMI                                                      \
  0x0F /**< Device identification register. [0, 0, 1, 1, 0, 0, 1, 1] */
#define LIS331_REG_CTRL1 0x20 ///< Power mode, data rate, axis enable
#define LIS331_REG_CTRL2 0x21 ///< Memory reboot, HPF config
#define LIS331_REG_CTRL3                                                       \
  0x22 ///< Interrupt config, poarity, pin mode, latching, pin enable
#define LIS331_REG_CTRL4 0x23           ///< BDU, Endianness, Range, SPI mode
#define LIS331_REG_CTRL5 0x24           ///< Sleep to wake enable
#define LIS331_REG_HP_FILTER_RESET 0x25 ///< Dummy register to reset filter
#define LIS331_REG_REFERENCE 0x26       ///< HPF reference value
#define LIS331_REG_STATUS 0x27  ///< Data overrun status, Data available status
#define LIS331_REG_OUT_X_L 0x28 /**< X-axis acceleration data. Low value */
#define LIS331_REG_OUT_X_H 0x29 /**< X-axis acceleration data. High value */
#define LIS331_REG_OUT_Y_L 0x2A /**< Y-axis acceleration data. Low value */
#define LIS331_REG_OUT_Y_H 0x2B /**< Y-axis acceleration data. High value */
#define LIS331_REG_OUT_Z_L 0x2C /**< Z-axis acceleration data. Low value */
#define LIS331_REG_OUT_Z_H 0x2D /**< Z-axis acceleration data. High value */
#define LIS331_REG_INT1CFG 0x30 ///< INT1 config. Enable on hi/low for each axis
#define LIS331_REG_INT1SRC 0x31 ///< INT1 source info
#define LIS331_REG_INT1THS 0x32 ///< INT1 acceleration threshold
#define LIS331_REG_INT1DUR 0x33 ///< INT1 duration threshold
#define LIS331_REG_INT2CFG 0x34 ///< INT2 config. Enable on hi/low for each axis
#define LIS331_REG_INT2SRC 0x35 ///< INT2 source info
#define LIS331_REG_INT2THS 0x36 ///< INT2 acceleration threshold
#define LIS331_REG_INT2DUR 0x37 ///< INT3 duration threshold

/** The high pass filter cutoff frequency */
typedef enum hpf_cutoff {
  LIS331_HPF_0_02_ODR,   ///< ODR/50
  LIS331_HPF_0_01_ODR,   ///< ODR/100
  LIS331_HPF_0_005_ODR,  ///< ODR/200
  LIS331_HPF_0_0025_ODR, ///< ODR/400
} lis331_hpf_cutoff_t;

/** The low pass filter cutoff frequency **/
typedef enum {
  LIS331_LPF_37_HZ,
  LIS331_LPF_74_HZ,
  LIS331_LPF_292_HZ,
  LIS331_LPF_780_HZ,

} lis331_lpf_cutoff_t;

/** Used with register 0x2A (LIS331HH_REG_CTRL_REG1) to set bandwidth **/
typedef enum {
  LIS331_DATARATE_POWERDOWN = 0,
  LIS331_DATARATE_50_HZ = 0x4,
  LIS331_DATARATE_100_HZ = 0x5,
  LIS331_DATARATE_400_HZ = 0x6,
  LIS331_DATARATE_1000_HZ = 0x7,
  LIS331_DATARATE_LOWPOWER_0_5_HZ = 0x8,
  LIS331_DATARATE_LOWPOWER_1_HZ = 0xC,
  LIS331_DATARATE_LOWPOWER_2_HZ = 0x10,
  LIS331_DATARATE_LOWPOWER_5_HZ = 0x14,
  LIS331_DATARATE_LOWPOWER_10_HZ = 0x18,
} lis331_data_rate_t;

/** A structure to represent axes **/
typedef enum {
  LIS331_AXIS_X = 0x0,
  LIS331_AXIS_Y = 0x1,
  LIS331_AXIS_Z = 0x2,
} lis331_axis_t;

/**
 * @brief Mode Options
 *
 */
typedef enum {
  LIS331_MODE_SHUTDOWN,
  LIS331_MODE_NORMAL,
  LIS331_MODE_LOW_POWER // Low power is from 2-6 so checks against this should
                        // be 'mode >=LIS331_MODE_LOW_POWER'
} lis331_mode_t;
/*!
 *  @brief  Class that stores state and functions for interacting with
 *          Adafruit_LIS331
 */
class Adafruit_LIS331 : public Adafruit_Sensor {
public:
  Adafruit_LIS331(TwoWire *Wi = &Wire);
  Adafruit_LIS331(int8_t cspin, SPIClass *theSPI = &SPI);
  Adafruit_LIS331(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);

  uint8_t getDeviceID(void);
  bool configIntDataReady(uint8_t irqnum = 1, bool activelow = true,
                          bool opendrain = true);

  void read(void);

  bool getEvent(sensors_event_t *event);
  void getSensor(sensor_t *sensor);
  void enableHighPassFilter(bool filter_enabled,
                            lis331_hpf_cutoff_t cutoff = LIS331_HPF_0_0025_ODR,
                            bool use_reference = false);
  void setHPFReference(int8_t reference);
  int8_t getHPFReference(void);
  void HPFReset(void);
  bool setLPFCutoff(lis331_lpf_cutoff_t cutoff);

  void setDataRate(lis331_data_rate_t dataRate);
  lis331_data_rate_t getDataRate(void);

  lis331_mode_t getMode(void);
  lis331_mode_t getMode(lis331_data_rate_t rate);
  int16_t x; /**< x axis value */
  int16_t y; /**< y axis value */
  int16_t z; /**< z axis value */

protected:
  float x_g; /**< x_g axis value (calculated by selected range) */
  float y_g; /**< y_g axis value (calculated by selected range) */
  float z_g; /**< z_g axis value (calculated by selected scale) */
  virtual void _scaleValues(void);

  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  Adafruit_SPIDevice *spi_dev = NULL; ///< Pointer to I2C bus interface

  void writeRange(uint8_t range);
  uint8_t readRange(void);

private:
  int32_t _sensorID;
};

#endif
