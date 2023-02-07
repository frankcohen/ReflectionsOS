/*
*****************************************************************************
@file         Deneyap_9EksenAtaletselOlcumBirimi.h
@mainpage     Deneyap 9 Dof IMU MMC5603NJ Arduino library header file
@version      v1.0.0
@date         June 23, 2022
@brief        This file contains all function prototypes and macros
              for Deneyap 9 Dof IMU MMC5603NJ Arduino library
*****************************************************************************
*/

#ifndef __MAGNETOMETER_H
#define __MAGNETOMETER_H

#include <Wire.h>
#include <Arduino.h>

#define Control_Reg_0 0x1B
#define Control_Reg_1 0x1C
#define Control_Reg_2 0x1D

/*
  Xout0 = 0x00,				// Xout[19:12]
  Xout1 = 0x01,				// Xout[11:4]
  Yout0 = 0x02,				// Yout[19:12]
  Yout1 = 0x03,				// Yout[11:4]
  Zout0 = 0x04,				// Zout[19:12]
  Zout1 = 0x05,				// Zout[11:4]
  Xout2 = 0x06,				// Xout[3:0]
  Yout2 = 0x07,				// Yout[3:0]
  Zout2 = 0x08,				// Zout[3:0]
  Tout = 0x09,				// Temperature output
  Status1 = 0x18,				// Device status1
  ODR = 0x1A,					// Output data rate
  InternalControl0 = 0x1B,	// Control register 0
  InternalControl1 = 0x1C,	// Control register 1
  InternalControl2 = 0x1D,	// Control register 2
  ST_X_TH = 0x1E,				// X-axis selftest threshold
  ST_Y_TH = 0x1F,				// Y-axis selftest threshold
  ST_Z_TH = 0x20,				// Z-axis selftest threshold
  ST_X = 0x27,				// X-axis selftest set value
  ST_Y = 0x28,				// Y-axis selftest set value
  ST_Z = 0x29,				// Z-axis selftest set value
  ProductID = 0x39,			// Product ID
  */


class MAGNETOMETER {
public:
    bool begin(uint8_t address, TwoWire &wirePort = Wire);
    void RegRead();
    int readMagnetometerX();
    int readMagnetometerY();
    int readMagnetometerZ();
    int readData();

private:
    uint8_t _address;
    uint8_t writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegisters(uint8_t address, uint8_t *data, size_t length);
};

#endif
