/*
  This is a library written for the ST VL53L5CX Time-of-flight sensor
  SparkFun sells these at its website:
  https://www.sparkfun.com/products/18642

  Do you like this library? Help support open source hardware. Buy a board!

  Written by Ricardo Ramos  @ SparkFun Electronics, October 22nd, 2021
  This file implements the VL53L5CX core library class.

  This library uses ST's VL53L5CX driver and parts of Simon Levy's VL53L5CX
  Arduino library available at https://github.com/simondlevy/VL53L5/tree/main/src/st

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SparkFun_VL53L5CX_Library.h"
#include "SparkFun_VL53L5CX_IO.h"
#include "vl53l5cx_api.h"

void SparkFun_VL53L5CX::clearErrorStruct()
{
    // Set last error struct to no-error condition
    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::NO_ERROR;
    lastError.lastErrorValue = 0;
}

bool SparkFun_VL53L5CX::begin(byte address, TwoWire &wirePort)
{
    clearErrorStruct();

    VL53L5CX_i2c = new SparkFun_VL53L5CX_IO();
    Dev = new VL53L5CX_Configuration();

    Dev->platform.VL53L5CX_i2c = VL53L5CX_i2c;

    bool ready = VL53L5CX_i2c->begin(address, wirePort);
    uint8_t result = 0;
    uint8_t deviceId = 0;
    uint8_t revisionId = 0;

    if (!ready)
    {
        SAFE_CALLBACK(errorCallback, SF_VL53L5CX_ERROR_TYPE::I2C_INITIALIZATION_ERROR, UNKNOWN_ERROR_VALUE);
        return false;
    }

    VL53L5CX_i2c->writeSingleByte(0x7fff, 0x00);
    deviceId = VL53L5CX_i2c->readSingleByte(0x00);
    revisionId = VL53L5CX_i2c->readSingleByte(0x01);
    VL53L5CX_i2c->writeSingleByte(0x7fff, 0x02);

    if ((revisionId != REVISION_ID) && (deviceId != DEVICE_ID))
    {
        lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_DEVICE;
        lastError.lastErrorValue = UNKNOWN_ERROR_VALUE;
        SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
        return false;
    }

    result = vl53l5cx_init(Dev);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::DEVICE_INITIALIZATION_ERROR;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

void SparkFun_VL53L5CX::setErrorCallback(void (*_errorCallback)(SF_VL53L5CX_ERROR_TYPE errorCode, uint32_t errorValue))
{
    errorCallback = _errorCallback;
}

bool SparkFun_VL53L5CX::isConnected()
{
    clearErrorStruct();

    uint8_t result = 0;

    bool connected = VL53L5CX_i2c->isConnected();

    if (!connected)
    {
        SAFE_CALLBACK(errorCallback, SF_VL53L5CX_ERROR_TYPE::I2C_NOT_RESPONDING, UNKNOWN_ERROR_VALUE);
        return false;
    }

    uint8_t alive = 0;
    result = vl53l5cx_is_alive(Dev, &alive);
    if (result != 0)
    {
        SAFE_CALLBACK(errorCallback, SF_VL53L5CX_ERROR_TYPE::DEVICE_NOT_ALIVE, result);
        return false;
    }
    else
    {
        return (alive != 0);
    }
}

bool SparkFun_VL53L5CX::setAddress(uint8_t newAddress)
{
    clearErrorStruct();

    // Don't use core vl53l5cx_set_i2c_address() as it calls WrByte with new address that SparkFun driver is not yet aware of
    //uint8_t result = vl53l5cx_set_i2c_address(Dev, static_cast<uint16_t>(newAddress));

    uint8_t result = VL53L5CX_i2c->writeSingleByte(0x7fff, 0x00);
    result |= VL53L5CX_i2c->writeSingleByte(0x4, newAddress);

    if (result == 0)
    {
        VL53L5CX_i2c->setAddress(newAddress); //Tell the middle layer what our new address is
        _i2cAddress = newAddress; //Tell Arduino lib what our new address is
        vl53l5cx_set_i2c_address(Dev, static_cast<uint16_t>(newAddress)); //Tell the core what our new address is
        
        result |= VL53L5CX_i2c->writeSingleByte(0x7fff, 0x02); //Do last write with new address
        
        return true;
    }

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_CHANGE_I2C_ADDRESS;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

uint8_t SparkFun_VL53L5CX::getAddress()
{
    return _i2cAddress;
}

bool SparkFun_VL53L5CX::setRangingFrequency(uint8_t newFrequency)
{
    clearErrorStruct();

    uint8_t result = vl53l5cx_set_ranging_frequency_hz(Dev, newFrequency);
    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_FREQUENCY_SETTING;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

uint8_t SparkFun_VL53L5CX::getRangingFrequency()
{
    clearErrorStruct();

    uint8_t frequency = 0;
    uint8_t result = vl53l5cx_get_ranging_frequency_hz(Dev, &frequency);
    if (result == 0)
    {
        return frequency;
    }
    else
    {
        lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_FREQUENCY_SETTING;
        lastError.lastErrorValue = static_cast<uint32_t>(result);
        SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
        return 0;
    }
}

bool SparkFun_VL53L5CX::setRangingMode(SF_VL53L5CX_RANGING_MODE rangingMode)
{
    clearErrorStruct();

    uint8_t result;

    if (rangingMode == SF_VL53L5CX_RANGING_MODE::AUTONOMOUS)
        result = vl53l5cx_set_ranging_mode(Dev, VL53L5CX_RANGING_MODE_AUTONOMOUS);
    else
        result = vl53l5cx_set_ranging_mode(Dev, VL53L5CX_RANGING_MODE_CONTINUOUS);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_RANGING_MODE;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

SF_VL53L5CX_RANGING_MODE SparkFun_VL53L5CX::getRangingMode()
{
    clearErrorStruct();

    uint8_t rangeMode = 0;
    uint8_t result = vl53l5cx_get_ranging_mode(Dev, &rangeMode);
    if (result == 0)
    {
        if (rangeMode == 0x01)
            return SF_VL53L5CX_RANGING_MODE::CONTINUOUS;
        else
            return SF_VL53L5CX_RANGING_MODE::AUTONOMOUS;
    }

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_RANGING_MODE;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return SF_VL53L5CX_RANGING_MODE::ERROR;
}

bool SparkFun_VL53L5CX::startRanging()
{
    clearErrorStruct();

    uint8_t result = vl53l5cx_start_ranging(Dev);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_START_RANGING;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

bool SparkFun_VL53L5CX::stopRanging()
{
    clearErrorStruct();

    uint8_t result = vl53l5cx_stop_ranging(Dev);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_STOP_RANGING;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

bool SparkFun_VL53L5CX::isDataReady()
{
    clearErrorStruct();
    uint8_t dataReady = 0;

    uint8_t result = vl53l5cx_check_data_ready(Dev, &dataReady);
    if (result == 0)
        return dataReady != 0;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_DATA_READY;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

uint8_t SparkFun_VL53L5CX::getResolution()
{
    clearErrorStruct();

    uint8_t resolution = 0;
    uint8_t result = vl53l5cx_get_resolution(Dev, &resolution);
    if (result == 0)
    {
        if (resolution == 64)
            return (uint8_t)SF_VL53L5CX_RANGING_RESOLUTION::RES_8X8;
        else
            return (uint8_t)SF_VL53L5CX_RANGING_RESOLUTION::RES_4X4;
    }

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_RESOLUTION;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return (uint8_t)SF_VL53L5CX_RANGING_RESOLUTION::ERROR;
}

bool SparkFun_VL53L5CX::setResolution(uint8_t resolution)
{
    clearErrorStruct();

    uint8_t result = vl53l5cx_set_resolution(Dev, resolution);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_SET_RESOLUTION;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

bool SparkFun_VL53L5CX::getRangingData(VL53L5CX_ResultsData *pRangingData)
{
    clearErrorStruct();

    uint8_t result = vl53l5cx_get_ranging_data(Dev, pRangingData);
    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_RANGING_DATA;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

bool SparkFun_VL53L5CX::setPowerMode(SF_VL53L5CX_POWER_MODE powerMode)
{
    clearErrorStruct();

    uint8_t powerModeValue;
    if (powerMode == SF_VL53L5CX_POWER_MODE::SLEEP)
        powerModeValue = VL53L5CX_POWER_MODE_SLEEP;
    else
        powerModeValue = VL53L5CX_POWER_MODE_WAKEUP;

    uint8_t result = vl53l5cx_set_power_mode(Dev, powerModeValue);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_SET_POWER_MODE;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

SF_VL53L5CX_POWER_MODE SparkFun_VL53L5CX::getPowerMode()
{
    clearErrorStruct();

    uint8_t powerModeValue;
    uint8_t result = vl53l5cx_get_power_mode(Dev, &powerModeValue);

    if (result == 0)
    {
        switch (powerModeValue)
        {
        case 0:
            return SF_VL53L5CX_POWER_MODE::SLEEP;
            break;

        case 1:
            return SF_VL53L5CX_POWER_MODE::WAKEUP;
            break;

        default:
            return SF_VL53L5CX_POWER_MODE::ERROR;
            break;
        }
    }

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_POWER_MODE;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return SF_VL53L5CX_POWER_MODE::ERROR;
}

bool SparkFun_VL53L5CX::setIntegrationTime(uint32_t timeMsec)
{
    clearErrorStruct();

    // Does not allow invalid values to be sent to the device
    if (timeMsec < 2 || timeMsec > 1000)
    {
        lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_INTEGRATION_TIME;
        lastError.lastErrorValue = UNKNOWN_ERROR_VALUE;
        SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
        return false;
    }

    uint8_t result = vl53l5cx_set_integration_time_ms(Dev, timeMsec);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_SET_INTEGRATION_TIME;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

uint32_t SparkFun_VL53L5CX::getIntegrationTime()
{
    clearErrorStruct();
    uint32_t integrationTime;
    uint8_t result = vl53l5cx_get_integration_time_ms(Dev, &integrationTime);

    if (result == 0)
    {
        return integrationTime;
    }

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_INTEGRATION_TIME;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return result;
}

bool SparkFun_VL53L5CX::setSharpenerPercent(uint8_t percent)
{
    clearErrorStruct();

    if (percent > 99)
    {
        lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::INVALID_SHARPENER_VALUE;
        lastError.lastErrorValue = UNKNOWN_ERROR_VALUE;
        SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    }

    uint8_t result = vl53l5cx_set_sharpener_percent(Dev, percent);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_SET_SHARPENER_VALUE;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

uint8_t SparkFun_VL53L5CX::getSharpenerPercent()
{
    clearErrorStruct();

    uint8_t percent;
    uint8_t result = vl53l5cx_get_sharpener_percent(Dev, &percent);

    if (result == 0)
        return percent;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_SHARPENER_VALUE;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return 0xff;
}

bool SparkFun_VL53L5CX::setTargetOrder(SF_VL53L5CX_TARGET_ORDER order)
{
    clearErrorStruct();

    uint8_t orderValue;
    if (order == SF_VL53L5CX_TARGET_ORDER::CLOSEST)
        orderValue = VL53L5CX_TARGET_ORDER_CLOSEST;
    else
        orderValue = VL53L5CX_TARGET_ORDER_STRONGEST;

    uint8_t result = vl53l5cx_set_target_order(Dev, orderValue);

    if (result == 0)
        return true;

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_SET_TARGET_ORDER;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return false;
}

SF_VL53L5CX_TARGET_ORDER SparkFun_VL53L5CX::getTargetOrder()
{
    clearErrorStruct();

    uint8_t orderValue;
    uint8_t result = vl53l5cx_get_target_order(Dev, &orderValue);

    if (result == 0)
    {
        if (orderValue == VL53L5CX_TARGET_ORDER_CLOSEST)
            return SF_VL53L5CX_TARGET_ORDER::CLOSEST;
        else
            return SF_VL53L5CX_TARGET_ORDER::STRONGEST;
    }

    lastError.lastErrorCode = SF_VL53L5CX_ERROR_TYPE::CANNOT_GET_TARGET_ORDER;
    lastError.lastErrorValue = static_cast<uint32_t>(result);
    SAFE_CALLBACK(errorCallback, lastError.lastErrorCode, lastError.lastErrorValue);
    return SF_VL53L5CX_TARGET_ORDER::ERROR;
}

uint8_t SparkFun_VL53L5CX::getWireMaxPacketSize()
{
    return VL53L5CX_i2c->getMaxPacketSize();
}

void SparkFun_VL53L5CX::setWireMaxPacketSize(uint8_t newSize)
{
    VL53L5CX_i2c->setMaxPacketSize(newSize);
}