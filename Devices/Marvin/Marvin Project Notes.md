# Marvin project notes, Reflections

Started: April 15, 2024

Updated: May 16, 2024

Marvin is the 10th revision of the Reflections main board. 
It is the successor to Yertle, Cat In Hat, BlueFish, RedFish, CindyLou, Horton, ThingTwo, Hoober, Knox and Sox.

- Sox - 1st generation board design
- Hoober - rev 2, fixes problems
- ThingTwo - rev 3, fixes problems
- Horton - rev 4, ready for software development
- CindyLou - rev 5, Beryl brighter display and component connectors
- RedFish - rev 6, Fixes LDO burn-out problem
- BlueFish - rev 7, Puts gesture sensor onto main board, moves USB port
- CatInHat - rev 8, Adds press-and-hold power on/off, removed in Yertle
- Yertle - rev 9, removes the power control in U9.
- Marvin - rev 10, Adds software Nand power control

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

Nand U5 is easily overwhelmed with requests. The controller stops responding. The Espressif provided SD library does not have enough logging to identify the problem, nor any logic to control the rate of interactions with the Nand controller. Only after a Nand power cycle does the Nand controller begin working again. We are solving this by adding custom logging and logic to othe SD library and in Marvin adding a software controlled method for Nand power cycling.

## Marvin Requirements

1. Remove U9

2. Use https://jlcpcb.com/partdetail/Mk-MKDV2GILAS/C726727 to increase Nand U11 storage to 2 gigabits

3. Add a software power control for the Nand U5

4. Parts substitutions. R3, R16, R20, R24, U11 are in 'shortfall' status from JLCPCB. Here are the replacements.

R3, https://jlcpcb.com/partdetail/Resi-HPCR0402F6K80K9/C365045
R16, R20, R24, https://jlcpcb.com/partdetail/Yageo-RC0402FR0710KL/C60490
U11, https://jlcpcb.com/partdetail/Mk-MKDV2GILAS/C726727

