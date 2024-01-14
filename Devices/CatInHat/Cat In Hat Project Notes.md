# Cat In Hat project notes, Reflections

Started: November 26, 2023

Updated: December 7, 2023

Cat In Hat is the 8th revision of the Reflections main board. 
It is the successor to BlueFish, RedFish, CindyLou, Horton, ThingTwo, Hoober, Knox and Sox. Cat In Hat adds press-and-hold Boot button to power on/off the board, improved SPI bus traces for the NAND, and notches in the board for the boot and reset buttons.

- Sox - 1st generation board design
- Hoober - rev 2, fixes problems
- ThingTwo - rev 3, fixes problems
- Horton - rev 4, ready for software development
- CindyLou - rev 5, Beryl brighter display and component connectors
- RedFish - rev 6, Fixes LDO burn-out problem
- BlueFish - rev 7, Puts gesture sensor onto main board, moves USB port

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

Find the CindyLou board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou

Find the RedFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/RedFish

Find the BlueFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/BlueFish

## What went wrong in Blue Fish

The NAND sometimes fails to write sectors, and then fails to mount. Sometimes removing power from it will get it to mount. It could also be the board traces for the NAND's SPI bus need to be tuned for high speed operation.

The board has no power-down capability.

The case to house the board needs easy push-button access to the Boot and Reset switches. The main board gets in the way.

The board needs to host the speaker and haptic motor to fit with a minimum of height into a wrist watch case.

## Cat In Hat Requirements

1. Add press-and-hold to the Boot button to turn power on to the board, and press-and-hold to turn power off. Add LTC 2954 pushbutton on/off controller.

2. Tune the SPI bus board traces for the NAND.

3. Add notches into the board below the Boot and Reset switches.

4. Change speaker to SW151008-1 and attach to board.

5. Make room for haptic motor to be attached to board.

6. Use 0.8 board thickness.



