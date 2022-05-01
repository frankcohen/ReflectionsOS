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

HOOBER REQUIREMENTS
-------------------

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

- Hirose DF65 Connector for speaker, haptic motor, battery wire connections
  https://www.hirose.com/product/series/DF65
  Hoober removes the Sox solder pads for speaker, haptic, gesture, battery, power, buttons
  Remove resistors for buttons.

- Power switch, press and hold to turn-on, press and hold to force off. Switch is
  side mounted at the edge of the board, at a 180 degree angle from the USB port.

- Battery charging when powered over USB-C. Powered by LiPo or USB.

- TFT welded to board, no connector.

- Board size is 34 mm round, no tab for USB port

- Gesture Sensor daughterboard and main board connector. Specs at:
  https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Hoober/Gesture%20Sensor%20Daughter%20Board.pdf
  Power to the TOF sensor programmable from one of the GPIOs

- JTAG pogo pin connectors to pins:
  TDI - GPIO 12
  TMS - GPIO 14
  TCLK - GPIO 13
  TDO - GPIO 15
  Single row Test programming fixture pogo pin pitch 2.54mm 8P/9P/10P/11P/ 3P~12P with wire
  https://www.ebay.com/itm/143924370628?var=443188826270

- USB port at 45 degrees

- Board clearance around ESP32-S3-Mini antenna

- Blue board color

Question? Ask principal maintainer Frank Cohen, fcohen@votsh.com
