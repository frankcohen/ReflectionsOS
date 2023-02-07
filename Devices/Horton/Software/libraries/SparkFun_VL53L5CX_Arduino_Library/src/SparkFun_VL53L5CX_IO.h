/*
  This is a library written for the ST VL53L5CX Time-of-flight sensor
  SparkFun sells these at its website:
  https://www.sparkfun.com/products/18642

  Do you like this library? Help support open source hardware. Buy a board!

  Written by Ricardo Ramos  @ SparkFun Electronics, October 22nd, 2021
  This file declares the VL53L5CX I2C driver class.

  This library uses ST's VL53L5CX driver and parts of Simon Levy's VL53L5CX
  Arduino library available at https://github.com/simondlevy/VL53L5/tree/main/src/st

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SparkFun_VL53L5CX_IO__
#define __SparkFun_VL53L5CX_IO__

#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_VL53L5CX_Library_Constants.h"

class SparkFun_VL53L5CX_IO
{
private:
  // I2C instance
  TwoWire *_i2cPort;

  // Sensor address
  uint8_t _address;

  // I2C maximum packet size
  uint8_t wireMaxPacketSize = I2C_BUFFER_SIZE;

public:
  // Default constructor
  SparkFun_VL53L5CX_IO(){};

  // Begin two wire interface
  bool begin(byte address, TwoWire &wirePort);

  // Return true if we get a reply from the I2C device.
  bool isConnected();

  // Update driver's I2C address
  void setAddress(uint8_t newAddress);

  // Read a single byte from a register.
  uint8_t readSingleByte(uint16_t registerAddress);

  // Write a single byte into a register.
  uint8_t writeSingleByte(uint16_t registerAddress, uint8_t value);

  // Read multiple bytes from a register into buffer byte array.
  uint8_t readMultipleBytes(uint16_t registerAddress, uint8_t *buffer, uint16_t bufferSize);

  // Write multiple bytes to register from buffer byte array.
  uint8_t writeMultipleBytes(uint16_t registerAddress, uint8_t *buffer, uint16_t bufferSize);

  // Get I2C maximum packet size
  uint8_t getMaxPacketSize() { return wireMaxPacketSize; }

  // Set I2C maximum packet size
  void setMaxPacketSize(uint8_t newSize) { wireMaxPacketSize = newSize; }
};

#endif