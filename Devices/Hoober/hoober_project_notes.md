Hoober project notes, Reflections

Hoober is the revision 2 of the Reflections main board. Hoober
fixes problems in the Sox board and gets closer to manufacturing.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, ready for manufacturing

Problems from the Sox board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Sox

What went wrong in Sox:

- Capacitor/resistor missing for ESP32 to be in firmware upload mode.

- Unknown Arduino IDE board specification for ESP32 S3.

- Unknown if the ESP32 S3's came with bootloader pre-burned.

- Too many pads for my shaky hands to solder.

- The 6 layer Avinidad version is 11 times more expensive than the 4 layer
Muhammad version. Both are made by PCBway. Product W433092AS1K2 (Avinidad)
cost $58 USD per board with 6 layers, 39.5x36.1mm, with 75 pads.
Product W356416AS1J3 (Muhammad) cost $5 USD per board with 34 mm x 36 mm,
4 layers, and 68 pads.

- 4PCB delivered 6 weeks late, cost 5 times more, and assembly missed
3 parts.

- Pandemic forced Shenzhen city quarantine, slowing delivery.

- IMU became unavailable and was replaced.

- ESP32 S3 was just being released and we waited 1 month for inventory.

- DHL lost a parts shipment to Arizona, USA based 4PCB.

- Sox Avinadad board TXCO power pads reversed.

Hoober requirements:

- Sketch upload with Arduino IDE 1.8.19 over Micro USB connector.

- Uses "Adafruit ESP32-S3 Feather No PSRAM" derived Arduino IDE board definition.
https://www.adafruit.com/product/5323
Schematic at
https://learn.adafruit.com/assets/110822
other files at:
https://learn.adafruit.com/adafruit-esp32-s2-feather/downloads

- Firmware uploads are automatic. Does not need to hold-down the Boot nor Reset
  to initiate Firmware Download mode for downloading firmware through the serial port.
  This is the real difference from Adafruit ESP32-S2 Feather.

- ESP32 S3 with Espressif bootloader pre-flashed.

- Power switch, press and hold to turn-on, press and hold to force off. Switch is
  side mounted at the edge of the board, at a 180 degree angle from the USB port.

- Battery charging when powered over USB-C. Powered by LiPo or USB.

- TFT welded to board, no more connector.

- Board size is 34 mm round, no table for USB port

- Gesture Sensor daughterboard and main board connector






Move USB 45 degrees
Flash sketches and bootloader through USB, without switch
Gesture sensor connector
Power to the TOF sensor programmable from one of the GPIOs
USB recharges battery
JTAG pogo pads
Board clearance around ESP32 antenna
Connector instead of pads: speaker, haptic motor, battery
  https://www.hirose.com/product/series/DF65     https://www.youtube.com/watch?v=JXliTh3bPm8
Side button power reset
Button pads change to GPIO pads, no resister
Blue board color
Readable silk labels (pad labels larger)







Question? Ask principal maintainer Frank Cohen, fcohen@votsh.com
