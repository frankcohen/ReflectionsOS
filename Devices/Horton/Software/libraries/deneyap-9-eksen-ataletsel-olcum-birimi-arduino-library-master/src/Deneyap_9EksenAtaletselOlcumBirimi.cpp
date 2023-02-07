/*
********************************************************************************
@file         Deneyap_9EksenAtaletselOlcumBirimi.cpp
@mainpage     Deneyap 9 Dof IMU MMC5603NJ Arduino library source file
@maintainer   RFtek Electronics <techsupport@rftek.com.tr>
@version      v1.0.0
@date         June 23, 2022
@brief        Includes functions to control Deneyap 9 Dof IMU MMC5603NJ
              Arduino library

Library includes:
--> Configuration functions
--> Data manipulation functions
--> I2C communication functions
********************************************************************************
*/

#include "Deneyap_9EksenAtaletselOlcumBirimi.h"
#include <Wire.h>

/**
  * @brief  I2C initialization and read check
  * @param  adress: Device adress
  * @retval None
**/
bool MAGNETOMETER::begin(uint8_t address, TwoWire &wirePort) {
Serial.print( "address = " );
Serial.println( address );
    _address = address >> 1;

    Serial.print( "address = ");
    Serial.println( address );

    Serial.print( "address >> 1 ");
    Serial.println( address >> 1 );

    Serial.print( "address << 1 ");
    Serial.println( address << 1 );

    Wire.begin(3,4);     // fcohen@starlingwatch.com, changed to allow for ThingTwo I2C pins

    Wire.beginTransmission(_address);
    Wire.write(0);

    if(!Wire.endTransmission())
    {
      return true;
    }
    return false;
}

/**
  * @brief
  * @param
  * @retval
**/
void MAGNETOMETER::RegRead() {
  writeRegister(Control_Reg_0, 0x21); // Initiate data acquisition
  writeRegister(Control_Reg_0, 0x08); // SET   DoSet = Bit3
  writeRegister(Control_Reg_0, 0x10); // RESET DoReset = Bit 4
  writeRegister(Control_Reg_0, 0x01); // Take Measure of Magnetic field, or TM_M bit

  //  Turn on continuousMode
  writeRegister(0x1A, 0x03);  // ODR set to frequency
  writeRegister(0x1B, 0x80);  // ODR set to frequency
  writeRegister(0x1D, 0x10);  // ODR set to frequency
}

/**
  * @brief
  * @param
  * @retval
**/
uint8_t MAGNETOMETER::writeRegister(uint8_t address, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(address);
    Wire.write(value);
    Wire.endTransmission();
    return 1;
}

/**
  * @brief
  * @param
  * @retval
**/
int MAGNETOMETER::readMagnetometerX() {
    int x = 0;
    long I2C_ValueX = 0;
    byte data[9];
    // 0x00, 0x01, 0x06
    readRegisters(0x00, (uint8_t *)data, sizeof(data));

    for (int i = 7; i >= 0; i--) {
        I2C_ValueX += pow(2, 12 + i) * bitRead(data[0], i);
    }
    for (int i = 7; i >= 0; i--) {
        I2C_ValueX += pow(2, 4 + i) * bitRead(data[1], i);
    }
    for (int i = 7; i >= 4; i--) {
        I2C_ValueX += pow(2, i) * bitRead(data[6], i);
    }
    x = (I2C_ValueX - 524288) * 0.00625;

    return x;
}

/**
  * @brief
  * @param
  * @retval
**/
int MAGNETOMETER::readMagnetometerY() {
    int y = 0;
    long I2C_ValueY = 0;
    byte data[9];
    readRegisters(0x00, (uint8_t *)data, sizeof(data));
    for (int i = 7; i >= 0; i--) {
        I2C_ValueY += pow(2, 12 + i) * bitRead(data[2], i);
    }
    for (int i = 7; i >= 0; i--) {
        I2C_ValueY += pow(2, 4 + i) * bitRead(data[3], i);
    }
    for (int i = 7; i >= 4; i--) {
        I2C_ValueY += pow(2, i) * bitRead(data[7], i);
    }
    y = (I2C_ValueY - 524288) * 0.00625;
    return y;
}

/**
  * @brief
  * @param
  * @retval
**/
int MAGNETOMETER::readMagnetometerZ() {
    int z = 0;
    long I2C_ValueZ = 0;
    byte data[9];
    readRegisters(0x00, (uint8_t *)data, sizeof(data));
    for (int i = 7; i >= 0; i--) {
        I2C_ValueZ += pow(2, 12 + i) * bitRead(data[4], i);
    }
    for (int i = 7; i >= 0; i--) {
        I2C_ValueZ += pow(2, 4 + i) * bitRead(data[5], i);
    }
    for (int i = 7; i >= 4; i--) {
        I2C_ValueZ += pow(2, i) * bitRead(data[8], i);
    }
    z = (I2C_ValueZ - 524288) * 0.00625;
    return z;
}

/**
  * @brief
  * @param
  * @retval
**/
uint8_t MAGNETOMETER::readRegisters(uint8_t address, uint8_t *data, size_t length) {
    Wire.beginTransmission(_address);
    Wire.write(address);

    if (Wire.endTransmission(false) != 0) {
        return 0;
    }

    if (Wire.requestFrom(_address, length) != length) {
        return 0;
    }

    uint8_t mdata = 0;
    for (size_t i = 0; i < length; i++) {
        //*data++ = Wire.read();
        mdata = Wire.read();
        Serial.print( mdata );
        Serial.print( ", " );
        *data++ = mdata;
    }
    Serial.println(" ");

    return 0;
}

/**
  * @brief
  * @param
  * @retval
**/
int MAGNETOMETER::readData() {
    int x = 0;
    int y = 0;
    int z = 0;

    long I2C_ValueX, I2C_ValueY, I2C_ValueZ;
    byte data[9];

    int a = readRegisters(0x00, (uint8_t *)data, sizeof(data));

    I2C_ValueX = 0;
    I2C_ValueY = 0;
    I2C_ValueZ = 0;
    for (int i = 7; i >= 0; i--) {
        I2C_ValueX += pow(2, 12 + i) * bitRead(data[0], i);
        I2C_ValueY += pow(2, 12 + i) * bitRead(data[2], i);
        I2C_ValueZ += pow(2, 12 + i) * bitRead(data[4], i);
    }
    for (int i = 7; i >= 0; i--) {
        I2C_ValueX += pow(2, 4 + i) * bitRead(data[1], i);
        I2C_ValueY += pow(2, 4 + i) * bitRead(data[3], i);
        I2C_ValueZ += pow(2, 4 + i) * bitRead(data[5], i);
    }
    for (int i = 7; i >= 4; i--) {
        I2C_ValueX += pow(2, i) * bitRead(data[6], i);
        I2C_ValueY += pow(2, i) * bitRead(data[7], i);
        I2C_ValueZ += pow(2, i) * bitRead(data[8], i);
    }
    x = (I2C_ValueX - 524288) * 0.00625;
    y = (I2C_ValueY - 524288) * 0.00625;
    z = (I2C_ValueZ - 524288) * 0.00625;
    Serial.print("X ekseni = ");
    Serial.println(x);
    Serial.print("Y ekseni = ");
    Serial.println(y);
    Serial.print("Z ekseni = ");
    Serial.println(z);

    Serial.println(" ");

    return true;
}
