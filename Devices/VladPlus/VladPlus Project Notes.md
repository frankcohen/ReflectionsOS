# VladPlus project notes, Reflections

Started: June 15, 2024

Updated: October 29, 2024

VladPlus is the 11th revision of the Reflections main board. 
It is the successor to Vlad, Yertle, Cat In Hat, BlueFish, RedFish, CindyLou, Horton, ThingTwo, Hoober, Knox and Sox. Replaces U4 with a higher efficiency power converter. The old one heated up when recharging the battery.

- Sox - 1st generation board design
- Hoober - rev 2, fixes problems
- ThingTwo - rev 3, fixes problems
- Horton - rev 4, ready for software development
- CindyLou - rev 5, Beryl brighter display and component connectors
- RedFish - rev 6, Fixes LDO burn-out problem
- BlueFish - rev 7, Puts gesture sensor onto main board, moves USB port
- CatInHat - rev 8, Adds press-and-hold power on/off, removed in Yertle
- Yertle - rev 9, Removes press-and-hold power, adds Nand software power control
- Vlad - rev 10, Enables the accelerometer to wake the processor from sleep

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

Find the Vlad board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Vlad

## What went wrong in Vlad

U4 heated the board when charging the battery.

## VladPlus Requirements

Replace U4 with XCL238B333D2-G, Non-Isolated DC/DC Converters Non-Isolated DC/DC Converters HiSAT-COT 1.5A Inductor Built-in PWM/PFM auto control Step-Down converter

