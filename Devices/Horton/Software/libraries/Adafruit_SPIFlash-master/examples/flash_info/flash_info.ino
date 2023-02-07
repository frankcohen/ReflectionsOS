// The MIT License (MIT)
// Copyright (c) 2019 Ha Thach for Adafruit Industries

#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

// Uncomment to run example with custom SPI and SS e.g with FRAM breakout
// #define CUSTOM_CS   A5
// #define CUSTOM_SPI  SPI

#if defined(CUSTOM_CS) && defined(CUSTOM_SPI)
  Adafruit_FlashTransport_SPI flashTransport(CUSTOM_CS, CUSTOM_SPI);

#elif defined(ARDUINO_ARCH_ESP32)
  // ESP32 use same flash device that store code.
  // SPIFlash can use partition table to detect fatfs partition
  // Therefore there is no need to specify the SPI and SS
  Adafruit_FlashTransport_ESP32 flashTransport;

#elif defined(ARDUINO_ARCH_RP2040)
  // RP2040 use same flash device that store code.
  // Therefore there is no need to specify the SPI and SS
  // Use default (no-args) constructor to be compatible with CircuitPython partition scheme
  Adafruit_FlashTransport_RP2040 flashTransport;

  // For generic usage: Adafruit_FlashTransport_RP2040(start_address, size)
  // If start_address and size are both 0, value that match filesystem setting in
  // 'Tools->Flash Size' menu selection will be used

#else
  // On-board external flash (QSPI or SPI) macros should already
  // defined in your board variant if supported
  // - EXTERNAL_FLASH_USE_QSPI
  // - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
  #if defined(EXTERNAL_FLASH_USE_QSPI)
    Adafruit_FlashTransport_QSPI flashTransport;

  #elif defined(EXTERNAL_FLASH_USE_SPI)
    Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

  #else
    #error No QSPI/SPI flash are defined on your board variant.h !
  #endif
#endif

Adafruit_SPIFlash flash(&flashTransport);


/*  If you want to use a specific flash device, for example for a custom built board, first look for it in Adafruit_SPIFlash\src\flash_devices.h
 *  If it isn't in there you need to create your own definition like the W25Q80DLX_EXAMPLE example below.
 *  These definitions need to be edited to match information on the data sheet of the flash device that you want to use.
 *  If you are not sure what the manufacture ID, memory type and capacity values should be, try running the sketch anyway and look at the serial output
 *  The flash device will report these values to you as a single hexadecimal value (the JDEC ID)
 *  For example, the first device on the list - the W25Q80DLX - will report its JDEC ID as 0xef4014, which is made of these three values:
 *  manufacturer_id = 0xef
 *  memory_type     = 0x40
 *  capacity        = 0x14
 *  With this macro properly defined you can then create an array of device definitions as shown below, this can include any from the list of devices in flash_devices.h, and any you define yourself here
 *  You need to update the variable on line 71 to reflect the number of items in the array
 *  You also need to uncomment line 84 and comment out line 81 so this array will be passed to the flash memory driver. 
 *
 *  Example of a user defined flash memory device:
    #define W25Q80DLX_EXAMPLE                                                \
      {                                                                      \
        .total_size = 1*1024*1024,                                           \
            .start_up_time_us = 5000, .manufacturer_id = 0xef,               \
        .memory_type = 0x40, .capacity = 0x14, .max_clock_speed_mhz = 80,    \
        .quad_enable_bit_mask = 0x02, .has_sector_protection = false,        \
        .supports_fast_read = true, .supports_qspi = true,                   \
        .supports_qspi_writes = false, .write_status_register_split = false, \
        .single_status_byte = false, .is_fram = false,                       \
      }
 */


/*
 * Create an array of data structures and fill it with the settings we defined above.
 * We are using two devices, but more can be added if you want.
 */
//static const SPIFlash_Device_t my_flash_devices[] = {
//    W25Q80DLX_EXAMPLE,
//};
/*
 * Specify the number of different devices that are listed in the array we just created. If you add more devices to the array, update this value to match.
 */
//const int flashDevices = 1;



// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(100);   // wait for native usb

  Serial.println("Adafruit Serial Flash Info example");
  flash.begin();

  //Using a flash device not already listed? Start the flash memory by passing it the array of device settings defined above, and the number of elements in the array.
  //flash.begin(my_flash_devices, flashDevices);

  Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");
}

void loop()
{
  // nothing to do
}
