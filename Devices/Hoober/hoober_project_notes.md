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

- IPX display ground connector reversed.

- ESP32 pin 23 is data native USB support, don't need an FTDI converter for USB.

- USB connector solder connections could not hold the connector to the board.


HOOBER REQUIREMENTS
-------------------

- Sketch upload with Arduino IDE 1.8.19 over USB C connector.

- Uses "Adafruit ESP32-S3 Feather No PSRAM" derived Arduino IDE board definition.
  https://www.adafruit.com/product/5323
  Schematic at
  https://learn.adafruit.com/assets/110822
  other files at:
  https://learn.adafruit.com/adafruit-esp32-s2-feather/downloads

- ESP32 S3 Mini with Espressif bootloader pre-flashed.

- Hirose DF65 Connector for speaker, haptic motor, battery wire connections
  https://www.hirose.com/product/series/DF65
  Hoober removes the Sox solder pads for speaker, haptic, gesture, battery, power, center/left/right buttons
  Remove resistors for center, left, right buttons.

- Power switch, press and hold to turn-on, press to force off. Switch is
  side mounted at the edge of the board, at a 180 degree angle from the USB port.

- Boot and Reset buttons buttons side mounted at the edge of the board.

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

- USB port at 45 degrees to the ESP

- Board clearance around ESP32-S3-Mini antenna

- Green board color

- Boards in panelization

NOTES
-----

Getting Arduino IDE to compile and upload on MacOS 12.3.1
change python to python3 in
$HOME/Library/Arduino15/packages/esp32/hardware/esp32/2.0.2/platform.txt
https://forum.arduino.cc/t/mac-os-update-killed-esp32-sketch/969580/8

esptool on MacOS 12.3.1 with Arduino IDE 1.8.19
~/Library/Arduino15/packages/esp32/tools/esptool_py/3.1.0/esptool

Adafruit Feather ESP32-S3 has The board definition includes several software switches in the Tools drop-down menu:

USB Mode - The board definition supports two types of USB interaction with your computer. USB-OTG is USB On-the-Go (OTG) allows two USB devices to talk to each other without requiring the services of a personal computer. "Hardware CDC and JTAG" implements a serial port (CDC) to implement the serial console, instead of using UART with an external USB-UART bridge chip, including flashing, debugger, and bi-direction serial communication. I choose USB-OTG.

USB CDC On Boot - USB Communication Device Class (CDC) implements the serial console, instead of using a UART with an external USB-UART FTDI or other programmer bridge chip. I choose Enabled.

USB Firmware MSC On Boot - Mass Storage Class (MSC) makes the Feather appear to be a USB storage device connected to your computer. I choose Disabled.

USB DFU On Boot - USB Device Firmware Upgrade (DFU) enables updating the firmware of a USB device. I choose Disabled.

With the above switches set I choose the board port in the Tools menu, open the Arduino IDE Serial Monitor, then click the Upload button. I press the Boot button on the board. Arduino IDE esptool.py flashes the sketch.

After the upload completes, I click the Boot button. I see the output correctly in the Serial Monitor.

Question? Ask principal maintainer Frank Cohen, fcohen@votsh.com
