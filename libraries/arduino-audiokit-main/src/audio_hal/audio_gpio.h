/**
 * @file audio_gpio.h
 * @author Phil Schatzmann
 * @brief GPIO related functionality
 * @date 2021-12-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include "AudioKitSettings.h"
#ifdef ARDUINO
#  include "Arduino.h"
#else
// Arduino definitions used by this framwork
#ifndef HIGH
#  define HIGH 1
#endif
#ifndef LOW
#  define LOW 0
#endif
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#ifdef __cplusplus
extern "C" {
#endif

void digitalWrite(int, int);
int digitalRead(int);
void pinMode(int, int);

#ifdef __cplusplus
}
#endif

#endif


#if defined(ESP32) || defined(AUDIOKIT_USE_IDF)  
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#ifndef ARDUINO
#  include "hal/gpio_types.h"
#endif

#else // no ESP32
#include <stdbool.h>

#define I2S_NUM_0 0
#define I2S_NUM_1 1
#define I2S_NUM_MAX I2S_NUM_1
#define I2C_NUM_0 0
#define I2C_NUM_1 1
#define BIT64(nr)               (1ULL << (nr))
#define BIT(nr)                 (1 << (nr))


typedef int i2s_port_t;
typedef int i2c_port_t;
typedef uint32_t TickType_t;

typedef enum {
    GPIO_NUM_NC = -1,    /*!< Use to signal not connected to S/W */
    GPIO_NUM_0 = 0,     /*!< GPIO0, input and output */
    GPIO_NUM_1 = 1,     /*!< GPIO1, input and output */
    GPIO_NUM_2 = 2,     /*!< GPIO2, input and output */
    GPIO_NUM_3 = 3,     /*!< GPIO3, input and output */
    GPIO_NUM_4 = 4,     /*!< GPIO4, input and output */
    GPIO_NUM_5 = 5,     /*!< GPIO5, input and output */
    GPIO_NUM_6 = 6,     /*!< GPIO6, input and output */
    GPIO_NUM_7 = 7,     /*!< GPIO7, input and output */
    GPIO_NUM_8 = 8,     /*!< GPIO8, input and output */
    GPIO_NUM_9 = 9,     /*!< GPIO9, input and output */
    GPIO_NUM_10 = 10,   /*!< GPIO10, input and output */
    GPIO_NUM_11 = 11,   /*!< GPIO11, input and output */
    GPIO_NUM_12 = 12,   /*!< GPIO12, input and output */
    GPIO_NUM_13 = 13,   /*!< GPIO13, input and output */
    GPIO_NUM_14 = 14,   /*!< GPIO14, input and output */
    GPIO_NUM_15 = 15,   /*!< GPIO15, input and output */
    GPIO_NUM_16 = 16,   /*!< GPIO16, input and output */
    GPIO_NUM_17 = 17,   /*!< GPIO17, input and output */
    GPIO_NUM_18 = 18,   /*!< GPIO18, input and output */
    GPIO_NUM_19 = 19,   /*!< GPIO19, input and output */
    GPIO_NUM_20 = 20,   /*!< GPIO20, input and output */
    GPIO_NUM_21 = 21,   /*!< GPIO21, input and output */
    GPIO_NUM_22 = 22,   /*!< GPIO22, input and output */
    GPIO_NUM_23 = 23,   /*!< GPIO23, input and output */
    GPIO_NUM_25 = 25,   /*!< GPIO25, input and output */
    GPIO_NUM_26 = 26,   /*!< GPIO26, input and output */
    GPIO_NUM_27 = 27,   /*!< GPIO27, input and output */
    GPIO_NUM_28 = 28,   /*!< GPIO28, input and output */
    GPIO_NUM_29 = 29,   /*!< GPIO29, input and output */
    GPIO_NUM_30 = 30,   /*!< GPIO30, input and output */
    GPIO_NUM_31 = 31,   /*!< GPIO31, input and output */
    GPIO_NUM_32 = 32,   /*!< GPIO32, input and output */
    GPIO_NUM_33 = 33,   /*!< GPIO33, input and output */
    GPIO_NUM_34 = 34,   /*!< GPIO34, input mode only */
    GPIO_NUM_35 = 35,   /*!< GPIO35, input mode only */
    GPIO_NUM_36 = 36,   /*!< GPIO36, input mode only */
    GPIO_NUM_37 = 37,   /*!< GPIO37, input mode only */
    GPIO_NUM_38 = 38,   /*!< GPIO38, input mode only */
    GPIO_NUM_39 = 39,   /*!< GPIO39, input mode only */
    GPIO_NUM_MAX,
/** @endcond */
} gpio_num_t;


typedef enum{
    I2C_MODE_SLAVE = 0,   /*!< I2C slave mode */
    I2C_MODE_MASTER,      /*!< I2C master mode */
    I2C_MODE_MAX,
} i2c_mode_t;


/// I2C pins
typedef struct{
    i2c_mode_t mode;     /*!< I2C mode */
    gpio_num_t sda_io_num;      /*!< GPIO number for I2C sda signal */
    gpio_num_t scl_io_num;      /*!< GPIO number for I2C scl signal */
    bool sda_pullup_en;  /*!< Internal GPIO pull mode for I2C sda signal*/
    bool scl_pullup_en;  /*!< Internal GPIO pull mode for I2C scl signal*/

    union {
        struct {
            uint32_t clk_speed;      /*!< I2C clock frequency for master mode, (no higher than 1MHz for now) */
        } master;                    /*!< I2C master config */
        struct {
            uint8_t addr_10bit_en;   /*!< I2C 10bit address mode enable for slave mode */
            uint16_t slave_addr;     /*!< I2C address for slave mode */
            uint32_t maximum_speed;  /*!< I2C expected clock speed from SCL. */
        } slave;                     /*!< I2C slave config */
    };
    uint32_t clk_flags;              /*!< Bitwise of ``I2C_SCLK_SRC_FLAG_**FOR_DFS**`` for clk source choice*/
} i2c_config_t;

/// I2S Pins
typedef struct {
    gpio_num_t bck_io_num;
    gpio_num_t ws_io_num;
    gpio_num_t data_out_num;
    gpio_num_t data_in_num;
} i2s_pin_config_t;

// SPI Configuration
typedef struct {
    gpio_num_t mosi_io_num;    ///< GPIO pin for Master Out Slave In (=spi_d) signal, or -1 if not used.
    gpio_num_t miso_io_num;    ///< GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used.
    gpio_num_t sclk_io_num;      ///< GPIO pin for SPI Clock signal, or -1 if not used.
    gpio_num_t quadwp_io_num;
    gpio_num_t quadhd_io_num;
} spi_bus_config_t;

/// SPI device configuration
typedef struct {
    int spics_io_num;               ///< CS GPIO pin for this device, or -1 if not used
} spi_device_interface_config_t;


#endif // ESP32