/**************************************************************************/
/*!
  @file     Adafruit_VCNL4010.cpp

  @mainpage Adafruit VCNL4010 Ambient Light/Proximity Sensor

  @section intro Introduction

  This is a library for the Aadafruit VCNL4010 proximity sensor breakout board
  ----> http://www.adafruit.com/products/466

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  @section author Author

  K. Townsend (Adafruit Industries)

  @section license License

  BSD (see license.txt)
*/
/**************************************************************************/

#include "Adafruit_VCNL4010.h"

/**************************************************************************/
/*!
    @brief  Instantiates a new VCNL4010 class
*/
/**************************************************************************/
Adafruit_VCNL4010::Adafruit_VCNL4010() {}

/**************************************************************************/
/*!
    @brief  Setups the I2C connection and tests that the sensor was found. If
   so, configures for 200mA IR current and 390.625 KHz.
    @param addr Optional I2C address (however, all chips have the same address!)
    @param theWire Optional Wire object if your board has more than one I2C
   interface
    @return true if sensor was found, false if not
*/
/**************************************************************************/
boolean Adafruit_VCNL4010::begin(uint8_t addr, TwoWire *theWire) {
  _i2caddr = addr;
  _wire = theWire;
  _wire->begin();

  uint8_t rev = read8(VCNL4010_PRODUCTID);
  // Serial.println(rev, HEX);
  if ((rev & 0xF0) != 0x20) {
    return false;
  }

  setLEDcurrent(20);             // 200 mA
  setFrequency(VCNL4010_16_625); // 16.625 readings/second

  write8(VCNL4010_INTCONTROL, 0x08);
  return true;
}

/**************************************************************************/
/*!
    @brief  Set the LED current.
    @param  current_10mA  Can be any value from 0 to 20, each number represents
   10 mA, so if you set it to 5, its 50mA. Minimum is 0 (0 mA, off), max is 20
   (200mA)
*/
/**************************************************************************/

void Adafruit_VCNL4010::setLEDcurrent(uint8_t current_10mA) {
  if (current_10mA > 20)
    current_10mA = 20;
  write8(VCNL4010_IRLED, current_10mA);
}

/**************************************************************************/
/*!
    @brief  Get the LED current
    @return  The value directly from the register. Each bit represents 10mA so 5
   == 50mA
*/
/**************************************************************************/

uint8_t Adafruit_VCNL4010::getLEDcurrent(void) { return read8(VCNL4010_IRLED); }

/**************************************************************************/
/*!
    @brief  Set the measurement signal frequency
    @param  freq Sets the measurement rate for proximity. Can be VCNL4010_1_95
   (1.95 measurements/s), VCNL4010_3_90625 (3.9062 meas/s), VCNL4010_7_8125
   (7.8125 meas/s), VCNL4010_16_625 (16.625 meas/s), VCNL4010_31_25 (31.25
   meas/s), VCNL4010_62_5 (62.5 meas/s), VCNL4010_125 (125 meas/s) or
   VCNL4010_250 (250 measurements/s)
*/
/**************************************************************************/

void Adafruit_VCNL4010::setFrequency(vcnl4010_freq freq) {
  write8(VCNL4010_PROXRATE, freq);
}

/**************************************************************************/
/*!
    @brief  Get proximity measurement
    @return Raw 16-bit reading value, will vary with LED current, unit-less!
*/
/**************************************************************************/

uint16_t Adafruit_VCNL4010::readProximity(void) {
  uint8_t i = read8(VCNL4010_INTSTAT);
  i &= ~0x80;
  write8(VCNL4010_INTSTAT, i);

  write8(VCNL4010_COMMAND, VCNL4010_MEASUREPROXIMITY);
  while (1) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_PROXIMITYREADY) {
      return read16(VCNL4010_PROXIMITYDATA);
    }
    delay(1);
  }
}

/**************************************************************************/
/*!
    @brief  Get ambient light measurement
    @return Raw 16-bit reading value, unit-less!
*/
/**************************************************************************/

uint16_t Adafruit_VCNL4010::readAmbient(void) {
  uint8_t i = read8(VCNL4010_INTSTAT);
  i &= ~0x40;
  write8(VCNL4010_INTSTAT, i);

  write8(VCNL4010_COMMAND, VCNL4010_MEASUREAMBIENT);
  while (1) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_AMBIENTREADY) {
      return read16(VCNL4010_AMBIENTDATA);
    }
    delay(1);
  }
}

/**************************************************************************/
/*!
    @brief  I2C low level interfacing
*/
/**************************************************************************/

// Read 1 byte from the VCNL4000 at 'address'
uint8_t Adafruit_VCNL4010::read8(uint8_t address) {
  uint8_t data;

  _wire->beginTransmission(_i2caddr);
#if ARDUINO >= 100
  _wire->write(address);
#else
  _wire->send(address);
#endif
  _wire->endTransmission();

  delayMicroseconds(170); // delay required

  _wire->requestFrom(_i2caddr, (uint8_t)1);

#if ARDUINO >= 100
  return _wire->read();
#else
  return _wire->receive();
#endif
}

// Read 2 byte from the VCNL4000 at 'address'
uint16_t Adafruit_VCNL4010::read16(uint8_t address) {
  uint16_t data;

  _wire->beginTransmission(_i2caddr);
#if ARDUINO >= 100
  _wire->write(address);
#else
  _wire->send(address);
#endif
  _wire->endTransmission();

  _wire->requestFrom(_i2caddr, (uint8_t)2);
  while (!_wire->available())
    ;
#if ARDUINO >= 100
  data = _wire->read();
  data <<= 8;
  while (!_wire->available())
    ;
  data |= _wire->read();
#else
  data = _wire->receive();
  data <<= 8;
  while (!_wire->available())
    ;
  data |= _wire->receive();
#endif

  return data;
}

// write 1 byte
void Adafruit_VCNL4010::write8(uint8_t address, uint8_t data) {
  _wire->beginTransmission(_i2caddr);
#if ARDUINO >= 100
  _wire->write(address);
  _wire->write(data);
#else
  _wire->send(address);
  _wire->send(data);
#endif
  _wire->endTransmission();
}
