# RedFish project notes, Reflections

Started: October 6, 2023

Updated: November 13, 2023

Blue Fish is the 7th revision of the Reflections main board. 
It is the successor to RedFish, CindyLou, Horton, ThingTwo, Hoober, Knox and Sox.
Blue Fish adds an LNA  chip so Reflections can use inexpensive and small passive GPS antennae. Predecessors require active GPS antenna. Battery, speaker, haptic motor are now connected by solder pads.

- Sox - 1st generation board design
- Hoober - rev 2, fixes problems
- ThingTwo - rev 3, fixes problems
- Horton - rev 4, ready for software development
- CindyLou - rev 5, Beryl brighter display and component connectors
- RedFish - rev 6, Fixes LDO burn-out problem

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

Find the CindyLou board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou

Find the RedFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/RedFish

## What went wrong in Red Fish

Battery powered circuit does not provide enough voltage. The voltage drop through Q3 Mosfet (FDN340P) is about 0.3V. The voltage drop through LDO is about 1V. When battery voltage is 3.3V, the output of LDO is 2V causing the display to not illuminate.

Compared Red Fish schematic to [Adafruit Feather ESP32-S3](https://learn.adafruit.com/adafruit-esp32-s3-feather/downloads) reference board. The voltage drop through mosfet and LDO is almost 0V. 

The wire-to-board connectors for battery, haptic, and speaker work well, and add expense, require a lot of pressure to close.

GPS passive antennas are less expensive and smaller in physical size than active antennas required by the GPS module.

FCP boards for the gesture sensor are too expensive.

## Blue Fish Requirements

1. Use the [LDO](https://www.digikey.com/en/products/detail/diodes-incorporated/AP2112K-3-3TRG1/4470746?s=N4IgTCBcDaIAQEEAKYCMqwGkC0BmAdLgCoBKA4qiALoC%2BQA) from the Adafruit Feather ESP32-S3.

2. Use the [Mosfet](https://www.digikey.com/en/products/detail/diodes-incorporated/DMG3415U-7/2052768) from the Adafruit Feather ESP32-S3.

3. Remove U7 resistor, it's not part of the Adafruit Feather ESP32-S3 reference board.

4. Add an LNA chip to the GPS module for passive antenna support. Keep the IPEX connector for now, though we may change to an on-board antenna or an FCP external connector.

5. Change the battery, haptic, and audio connectors to solder pads. Replaces the previous 009176002884906 KYOCERA AVX wire-to-board connectors.

6. Update the silk: Starling Watch 2023, BlueFish T4 AM MAG. T4 is for Thi Thu Thao Tran, a talented electronics engineer who made the changes for Blue Fish.

7. Do not place GPIO header.

8. Remove the gesture sensor connector and place a VL53L5CX Time Of Flight sensor to a new tab on the main board.

9. Move the USB connector to the 3 o'clock position relative to the top side of the main board.

10. Add a slot/hole through the board for the display FCP connection. Relocate the display connector.
