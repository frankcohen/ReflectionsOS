Sox project notes

I need the EasyEDA schematic and layout for the Sox board
This is a 36 mm round board, probably 4 or more layers,
ESP32 Arduino-compatible design. Components are mounted top and bottom.

Sox is the main logic board for a new open-source hardware
project. Reflections will power entertainment and mobile
experiences.

I put files, photos of the breadboard, software, wiring guide into
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Sox

Requirements for this project:
1) Schematic, Layout, and BOM developed in EasyEDA Designer https://easyeda.com/editor

2) 36 mm diameter round board, designers choice on number of layers, traces are
1 oz copper thickness, 1.6 mm board thickness, components mounted on both
sides.

3) The breadboard for this project uses break out boards. These
have a bunch of step-up/down 3.3-5 volt adaptors, none of these are
necessary for the board. Everything needs to fit onto the 36 mm diameter
round board, except for the GPS antenna, TFT display, gesture sensor,
and battery. The board MUST use parts that are widely/easily available,
determined by checking stock and availability levels in LCSC.COM or
Digikey.com

4) Photos of the breadboard are at:
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Sox/Breadboard%20Photos

5) Board is powered by 1 lithium ion battery: 3.7 volts, 500 mAh.
Connector Type: 2P PH 2.0mm Pitch; Cable Length: 5cm / 83"
All components are on 3.3.

6) The board needs a uart, using the same provided on the HiLetGo ESP32 dev board
CP2102-GM. https://lcsc.com/product-detail/USB-ICs_SILICON-LABS-CP2102-GMR_C6568.html
USB connector at 180 degrees from ESP32 antennae for layout.

7) GPS module uses AT6558 running 3.3 volts. Breadboard shows connected to 5 volts
due to a requirement of the GPS breakout board. The breadboard uses the GPS + BDS
BeiDou Dual Module Flight Control Wiki - DFRobot breakout board. The board uses
only the AT6558 chip. And the board needs to provide an IPEX connector for the usb antenna.
When I put the board into a wrist watch enclosure the GPS antenna will go into the wrist band.
https://www.digikey.com/en/products/detail/suntsu-electronics,-inc./SCNRF-6AAN-P1FS/14291136?utm_adgroup=Suntsu%20Electronics%20Inc&utm_source=google&utm_medium=cpc&utm_campaign=Shopping_DK%2BSupplier_Other&utm_term=&utm_content=Suntsu%20Electronics%20Inc&gclid=CjwKCAiAxJSPBhAoEiwAeO_fP0jt6Ce-0s7Bdl8Msl-FxEtlkEP35RCr5qoYZS6azBifDDCklZcshhoCEmIQAvD_BwE

8) Gesture sensor (APDS-9960) is not mounted on the board, it is connected via
4 wires (VCC, Gnd, SCL, SDA), board needs 4 pads for soldering the wires.

9) Display connects directly to 13-pin .7 mm FPC connector on board,
no adaptor board, 1.28 inch lcd round is a 240x240 display using GC9A01 driver HD ips.
https://pan.baidu.com/s/1x9B9jKrjikSCBUI38ZUC3w Extraction code: 8888
https://www.taobao.com/list/item/606659413574.htm?spm=a21wu.12321156.recommend-tpp.1

10) Breadboard uses an ESP32 WROOM Development Board. To minimize space Requirements
the board may use the ESP32-S3-MINI-1.
https://www.espressif.com/en/products/modules

11) FTDI pads for pogo pins to flash the ESP32 bootloader.

12) PCB design best practices: decoupling capacitors, landing patterns large enough
for the footprint, avoid too many via holes, avoid excessive trace lengths (especially
in the SPI bus between TFT, SD, and ESP32), avoid EMI from the ESP32 antennae, and
expect board revisions to correct unforseen problems.

13) Uses ESP32 processors with pre-burned ESP-IDF V3.1 bootloader. The boards must
be verified by connecting a USB cable to a laptop running Arduino IDE 1.8 or later
and flash a testing sketch.
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/bootloader.html

Question? Ask principal maintainer Frank Cohen, fcohen@votsh.com
