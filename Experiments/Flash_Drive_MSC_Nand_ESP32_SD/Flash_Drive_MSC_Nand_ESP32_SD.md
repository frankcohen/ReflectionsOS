# Using the Reflections board NAND as a Flash Drive, FlashDrive + MSC + Nand + ESP32 + SD

November 16, 2023

Operates the Reflections board NAND/SD as a USB flash drive

Based on code from [@atomic14](https://www.youtube.com/@atomic14) and [SD MSC utility for ESP32 processors](https://github.com/atomic14/esp32-sdcard-msc/tree/main) 

Arduino IDE 2 sketch is [/Experiments/Flash_Drive_MSC_Nand_ESP32_SD](https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Flash_Drive_MSC_Nand_ESP32_SD/NANDMSC)

Arduino IDE settings:
- Adafruit Feather ESP32-S3 No PSRAM
- USB mode: USB-OTG
- USB CDC On Boot: Enabled
- USB Firmware MSC On Boot: Disabled
- USB DFU On Boot: Disabled
- JTAG Adapter: Integrated USB JTAG

![Schematic](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Flash_Drive_MSC_Nand_ESP32_SD/ESP32S3_NAND_USB_schematic.jpg)

![Screen shot](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Flash_Drive_MSC_Nand_ESP32_SD/Nand_Flash.jpg)

