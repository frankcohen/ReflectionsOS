# Vlad project notes, Reflections

Started: June 15, 2024

Updated: June 15, 2024

Vlad is the 10th revision of the Reflections main board. 
It is the successor to Yertle, Cat In Hat, BlueFish, RedFish, CindyLou, Horton, ThingTwo, Hoober, Knox and Sox. Vlad enables the accelerometer to interrupt the ESP32, including from deep sleep.

- Sox - 1st generation board design
- Hoober - rev 2, fixes problems
- ThingTwo - rev 3, fixes problems
- Horton - rev 4, ready for software development
- CindyLou - rev 5, Beryl brighter display and component connectors
- RedFish - rev 6, Fixes LDO burn-out problem
- BlueFish - rev 7, Puts gesture sensor onto main board, moves USB port
- CatInHat - rev 8, Adds press-and-hold power on/off, removed in Yertle
- Yertle - rev 9, Removes press-and-hold power, adds Nand software power control

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

Find the CindyLou board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou

Find the RedFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/RedFish

Find the BlueFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/BlueFish

Find the Cat In Hat board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CatInHat

Find the Yertle board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Yertle

## What went wrong in Yertle

Added U9 (LTC2954CTS8-1#TRPBF) to provide press-and-hold power on/off. U9 interferes with the ESP32-S3 EN operation. The processor will not go into download/bootloader mode.

## Vlad Requirements

1. Connect accelerometer pin 11 and 12 (Interrupt 1 and 2) to ESP32 GPIO 13 and 14. Enables accelerometer to wake ESP32-S3 from deep sleep on movement.


