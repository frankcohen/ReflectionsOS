// The MIT License (MIT)
// Copyright (c) 2019 Ha Thach for Adafruit Industries

#include "Adafruit_SPIFlash.h"

// Uncomment to run example with custom SPI and SS e.g with FRAM breakout
// #define CUSTOM_CS   A5
// #define CUSTOM_SPI  SPI

#if defined(CUSTOM_CS) && defined(CUSTOM_SPI)
  Adafruit_FlashTransport_SPI flashTransport(CUSTOM_CS, CUSTOM_SPI);

#elif defined(ARDUINO_ARCH_ESP32)
  // ESP32 use same flash device that store code.
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

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb

  flash.begin();
  
  Serial.println("Adafruit Serial Flash Sector Dump example");
  Serial.print("JEDEC ID: "); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.println(flash.size());
}

void dump_sector(uint32_t sector)
{
  uint8_t buf[4096];
  memset(buf, 0xff, sizeof(buf));
  
  flash.readBuffer(sector*4096, buf, 4096);

  for(uint32_t row=0; row<sizeof(buf)/16; row++)
  {
    if ( row == 0 ) Serial.print("0");
    if ( row < 16 ) Serial.print("0");
    Serial.print(row*16, HEX);
    Serial.print(" : ");

    for(uint32_t col=0; col<16; col++)
    {
      uint8_t val = buf[row*16 + col];

      if ( val < 16 ) Serial.print("0");
      Serial.print(val, HEX);

      Serial.print(" ");
    }

    Serial.println();
  }
}

void loop()
{
  Serial.print("Enter the sector number to dump: ");
  while( !Serial.available() ) delay(10);

  int sector = Serial.parseInt();

  Serial.println(sector); // echo

  if ( sector < flash.size()/4096 )
  {
    dump_sector(sector);
  }else
  {
    Serial.println("Invalid sector number");
  }

  Serial.println();
  delay(10); // a bit of delay
}
