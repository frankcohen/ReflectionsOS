# Reflections Devices ReadMe

Started: October 6, 2023

Updated: March 19, 2024

Reflections is a hardware, software, and server architecture for building mobile entertaining experiences. We went through 8 revisions to the board to get to a finished prototype. Each revision is in a sub directory of [Reflections/devices](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices).

- [Sox](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Sox) - 1st generation board design
- [Hoober](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Hoober) - rev 2, fixes problems
- [ThingTwo](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/ThingTwo) - rev 3, fixes problems
- [Horton](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton) - rev 4, ready for software development
- [CindyLou](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou) - rev 5, Beryl brighter display and component connectors
- [RedFish](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/RedFish) - rev 6, Fixes LDO burn-out problem, battery operated works
- [Blue Fish](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/BlueFish) - 7th revision, passive GPS antenna support, solder pads instead of connectors, gesture sensor on board, reposition USB and display tab
- [Cat In Hat](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CatInHat) - 8th revision, adding press-and-hold power on/off. This failed, turns out the ESP32-S3 doesn't like repurposing the EN pin to provide power on/off capability. Tried changing capacitor and resistor values for the EN circuit in a project codenamed Lorax. That did not work either.
- [Yertle](https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CatInHat) - 9th revision, removes U9 to abandon the press-and-hold power on/off.

![Cindy Lou board example](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/CindyLou.jpg)
