Sox project notes

I need the EasyEDA skematic and layout for the Sox board
This is a 36 mm round board, probably 4 or more layers,
ESP32 Arduino-compatible design. Components are mounted top and bottom.

Sox is the main logic board for a new open-source hardware
project. Reflections will power entertainment and mobile
experiences.

I put files, photos of the breadboard, software, wiring guide into
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Sox

Requirements for this project:
1) Schematic, Layout, and BOM developed in EasyEDA Designer https://easyeda.com/editor

2) 36 mm round board, designers choice on number of layers, traces are
1 oz copper thickness, 1.6 mm board thickness, components mounted on both
sides

3) Board is powered by 1 lithium ion battery: 3.7 volts, 500 mAh.
Connector Type: 2P PH 2.0mm Pitch; Cable Length: 5cm / 83"
All components are on 3.3.

4) GPS module uses AT6558 running 3.3 volts. Breadboard shows connected to 5 volts
due to a requirement of the GPS breakout board.

5) Gesture sensor (APDS-9960) is not mounted on the board, it is connected via
4 wires (VCC, Gnd, SCL, SDA), board needs 4 pads for soldering the wires.

6) Display connects directly to 13-pin .7 mm FPC connector on board,
no adaptor board, 1.28 inch lcd round is a 240x240 display using GC9A01 driver HD ips
https://pan.baidu.com/s/1x9B9jKrjikSCBUI38ZUC3w Extraction code: 8888
https://www.taobao.com/list/item/606659413574.htm?spm=a21wu.12321156.recommend-tpp.1

7) Breadboard uses an ESP32 WROOM Development Board. To minimize space Requirements
the board may use the ESP32-S3-MINI-1.
https://www.espressif.com/en/products/modules

8) USB connector at 180 degrees from ESP32 antennae.

9) FTDI pads for pogo pins to flash the ESP32 bootloader.

10) PCB design best practices: decoupling capacitors, landing patterns large enough
for the footprint, avoid too many via holes, avoid excessive trace lengths (especially
in the SPI bus between TFT, SD, and ESP32), avoid EMI from the ESP32 antennae, and
expect board revisions to correct unforseen problems.

11) Uses ESP32 processors with pre-burned ESP-IDF V3.1 bootloader. The boards must
be verified by connecting a USB cable to a laptop running Arduino IDE 1.8 or later
and flash a testing sketch.
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/bootloader.html

Question? Ask principal maintainer Frank Cohen, fcohen@votsh.com
