# Yertle project notes, Reflections

Started: February 12, 2024

Updated: February 14, 2024

Yertle is the 9th revision of the Reflections main board. 
It is the successor to Cat In Hat, BlueFish, RedFish, CindyLou, Horton, ThingTwo, Hoober, Knox and Sox. Cat In Hat added press-and-hold Boot button to power on/off the board, and it didn't work. The ESP32-S3 would not boot with the EN pin providing BOOT and power on/off controls. Yertle removes U9.

- Sox - 1st generation board design
- Hoober - rev 2, fixes problems
- ThingTwo - rev 3, fixes problems
- Horton - rev 4, ready for software development
- CindyLou - rev 5, Beryl brighter display and component connectors
- RedFish - rev 6, Fixes LDO burn-out problem
- BlueFish - rev 7, Puts gesture sensor onto main board, moves USB port
- CatInHat - rev 8, Adds press-and-hold power on/off, removed in Yertle

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

## What went wrong in Cat In Hat (and Lorax)

Added U9 (LTC2954CTS8-1#TRPBF) to provide press-and-hold power on/off. U9 interferes with the ESP32-S3 EN operation. The processor will not go into download/bootloader mode.

## Yertle Requirements

1. Remove U9.

