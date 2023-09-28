
/*
  This is a library written for the ST VL53L5CX Time-of-flight sensor
  SparkFun sells these at its website:
  https://www.sparkfun.com/products/18642

  Do you like this library? Help support open source hardware. Buy a board!

  Written by Ricardo Ramos  @ SparkFun Electronics, October 22nd, 2021
  This file is the core of the VL53L5CX ToF sensor library.

  This library uses ST's VL53L5CX driver and parts of Simon Levy's VL53L5CX
  Arduino library available at https://github.com/simondlevy/VL53L5/tree/main/src/st

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SparkFun_VL53L5CX_Library__
#define __SparkFun_VL53L5CX_Library__

#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_VL53L5CX_Library_Constants.h"
#include "SparkFun_VL53L5CX_IO.h"
#include "vl53l5cx_api.h"

struct SparkFun_VL53L5CX_Error
{
    // More descriptive error than a single uint32 value.
    SF_VL53L5CX_ERROR_TYPE lastErrorCode = SF_VL53L5CX_ERROR_TYPE::NO_ERROR;

    // Value as reported back from the sensor.
    uint32_t lastErrorValue = 0;
};

class SparkFun_VL53L5CX
{
private:
    // Sensor's I2C address.
    uint8_t _i2cAddress = 0;

    // Error callback function pointer.
    // Function must accept a SF_VL53L5CX_ERROR_TYPE as errorCode and an uint32_t as errorValue.
    void (*errorCallback)(SF_VL53L5CX_ERROR_TYPE errorCode, uint32_t errorValue) = nullptr;

    // Clears the error struct to a no-error state.
    void clearErrorStruct();

public:
    SparkFun_VL53L5CX_IO* VL53L5CX_i2c; // I2C driver object
    VL53L5CX_Configuration* Dev;        // Sensor condfiguration struct

    // This struct holds the last error which happened (if any).
    SparkFun_VL53L5CX_Error lastError;

    // Default empty constructor.
    SparkFun_VL53L5CX(){};

    // Start up the sensor. Passing an address and Wire port instance is optional.
    bool begin(byte address = (DEFAULT_I2C_ADDR >> 1), TwoWire &wirePort = Wire);

    // Set the error callback function.
    void setErrorCallback(void (*errorCallback)(SF_VL53L5CX_ERROR_TYPE errorCode, uint32_t errorValue));

    // Returns true if the sensor is connected and replying the correct device ID and revision ID.
    bool isConnected();

    // Returns true if the sensor's address was correctly changed or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setAddress(uint8_t newAddress);

    // Return the board address.
    uint8_t getAddress();

    // Returns true if the sensor's sampling frequency was changed accordingly or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setRangingFrequency(uint8_t newFrequency);

    // Returns current ranging frequency.
    // If this function returns 0 an error entry will be stored in the lastError struct.
    uint8_t getRangingFrequency();

    // Returns true if the sensor's ranging mode was changed accordingly or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setRangingMode(SF_VL53L5CX_RANGING_MODE rangingMode);

    // Returns the current ranging mode.
    SF_VL53L5CX_RANGING_MODE getRangingMode();

    // Returns true if the start ranging command was acknowledged by the sensor or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool startRanging();

    // Returns true if the stop ranging command was acknowledged by the sensor or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool stopRanging();

    // Returns true if data is ready.
    bool isDataReady();

    // Returns the current ranging resolution.
    uint8_t getResolution();

    // Returns true if the sensor's ranging resolution was changed accordingly or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setResolution(uint8_t resolution);

    // Returns true if the ranging data was read from the sensor or false otherwise.
    // Data will be stored in the VL53L5CX_ResultsData struct passed as a pointer.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool getRangingData(VL53L5CX_ResultsData *pRangingData);

    // Returns true if the sensor's power mode was changed accordingly or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setPowerMode(SF_VL53L5CX_POWER_MODE powerMode);

    // Returns the current power mode.
    // If this function returns SF_VL53L5CX_POWER_MODE::ERROR an error entry will be stored in the lastError struct.
    SF_VL53L5CX_POWER_MODE getPowerMode();

    // Returns true if the sensor's integration time was changed accordingly or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setIntegrationTime(uint32_t timeMsec);

    // Returns the integration time.
    // If this function returns 0 an error entry will be stored in the lastError struct.
    uint32_t getIntegrationTime();

    // Returns true if the sensor's integration time was changed accordingly or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setSharpenerPercent(uint8_t percent);

    // Get the sharpener percentage, ranging from 0 (disabled) to 99 (maximum).
    // If this function returns 0xff an error entry will be stored in the lastError struct.
    uint8_t getSharpenerPercent();

    // Returns true if the target order was set or false otherwise.
    // If this function returns false an error entry will be stored in the lastError struct.
    bool setTargetOrder(SF_VL53L5CX_TARGET_ORDER order);

    // Gets target order.
    // If this function returns SF_VL53L5CX_TARGET_ORDER::ERROR an error entry will be stored in the lastError struct.
    SF_VL53L5CX_TARGET_ORDER getTargetOrder();

    // Gets I2C maximum packet size.
    uint8_t getWireMaxPacketSize();

    // Sets I2C maximum packet size.
    void setWireMaxPacketSize(uint8_t newSize = I2C_BUFFER_SIZE);
};
#endif