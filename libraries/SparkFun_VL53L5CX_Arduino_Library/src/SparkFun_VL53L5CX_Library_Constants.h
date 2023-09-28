#ifndef __SparkFun_VL53L5CX_Library_Constants__
#define __SparkFun_VL53L5CX_Library_Constants__

#include <stdint.h>

// Macro for invoking the callback if the function pointer is valid
#define SAFE_CALLBACK(cb, code, value) \
    if (cb != nullptr)                 \
    {                                  \
        cb(code, value);               \
    }

// Constants declarations

const uint8_t DEFAULT_I2C_ADDR = 0x52;
const uint8_t I2C_BUFFER_SIZE = 32;
const uint8_t REVISION_ID = 0x02;
const uint8_t DEVICE_ID = 0xf0;
const uint32_t UNKNOWN_ERROR_VALUE = 0xffffffff;

// Error enumeration for callback usage
enum class SF_VL53L5CX_ERROR_TYPE : uint8_t
{
    NO_ERROR,
    I2C_INITIALIZATION_ERROR,
    I2C_NOT_RESPONDING,
    DEVICE_INITIALIZATION_ERROR,
    INVALID_DEVICE,
    INVALID_FREQUENCY_SETTING,
    INVALID_RANGING_MODE,
    CANNOT_CHANGE_I2C_ADDRESS,
    CANNOT_START_RANGING,
    CANNOT_STOP_RANGING,
    CANNOT_GET_DATA_READY,
    CANNOT_GET_RESOLUTION,
    CANNOT_GET_RANGING_DATA,
    CANNOT_SET_RESOLUTION,
    CANNOT_SET_POWER_MODE,
    CANNOT_GET_POWER_MODE,
    DEVICE_NOT_ALIVE,
    INVALID_INTEGRATION_TIME,
    CANNOT_SET_INTEGRATION_TIME,
    CANNOT_GET_INTEGRATION_TIME,
    INVALID_SHARPENER_VALUE,
    CANNOT_SET_SHARPENER_VALUE,
    CANNOT_GET_SHARPENER_VALUE,
    CANNOT_SET_TARGET_ORDER,
    CANNOT_GET_TARGET_ORDER,
    INVALID_TARGET_ORDER,
    UNKNOWN_ERROR
};

// ERROR values will be reported back upon retrieving a specific setting from the sensor
// if this setting is invalid. Code will trap and default to a non-error condition if the user
// tries to pass an ERROR enum member to any function which accepts it as an argument

enum class SF_VL53L5CX_RANGING_MODE : uint8_t
{
    CONTINUOUS,
    AUTONOMOUS,
    ERROR
};

enum class SF_VL53L5CX_RANGING_RESOLUTION : uint8_t
{
    RES_4X4 = 16,
    RES_8X8 = 64,
    ERROR
};

enum class SF_VL53L5CX_POWER_MODE : uint8_t
{
    SLEEP,
    WAKEUP,
    ERROR
};

enum class SF_VL53L5CX_TARGET_ORDER : uint8_t
{
    CLOSEST,
    STRONGEST,
    ERROR
};

#endif