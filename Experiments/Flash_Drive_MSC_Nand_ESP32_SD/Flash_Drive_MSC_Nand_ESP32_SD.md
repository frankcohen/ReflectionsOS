# Using the Reflections board NAND as a Flash Drive, FlashDrive + MSC + Nand + ESP32 + SD

November 16, 2023

Operates the Reflections board NAND/SD as a USB flash drive

Based on code from [@atomic14](https://www.youtube.com/@atomic14) and [SD MSC utility for ESP32 processors](https://github.com/atomic14/esp32-sdcard-msc/tree/main) 

Arduino IDE settings:
Adafruit Feather ESP32-S3 No PSRAM
USB mode: USB-OTG
USB CDC On Boot: Enabled
USB Firmware MSC On Boot: Disabled
USB DFU On Boot: Disabled




Reflections is a hardware, software, and server architecture for building mobile entertaining experiences. We went through 8 revisions to the board to get to a finished prototype. Each revision is in a sub directory of [Reflections/devices](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices).

- [Sox](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Sox) - 1st generation board design
- [Hoober](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Hoober) - rev 2, fixes problems
- [ThingTwo](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/ThingTwo) - rev 3, fixes problems
- [Horton](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton) - rev 4, ready for software development
- [CindyLou](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou) - rev 5, Beryl brighter display and component connectors
- [RedFish](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/RedFish) - rev 6, Fixes LDO burn-out problem, battery operated works
- [Blue Fish](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/BlueFish) - 7th revision, passive GPS antenna support, solder pads instead of connectors, gesture sensor on board, reposition USB and display tab

![Cindy Lou board example](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/CindyLou.jpg)
