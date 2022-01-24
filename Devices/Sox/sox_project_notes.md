Sox project notes, Reflections

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
All components are on 3.3 volts.
The board needs a battery sensor, a voltage divider connected to GPIO 2
to measure the battery voltage. Unlike the usual ESP32 development boards
there is no LED on this board.
The board needs to operate while USB is connected and charging the battery.
I anticipate users plugging in a USB powerbank to keep the watch going as
they play games on it.

6) GPIO 36 (right button), 39 (center button), 34 (left button) are pads on the
board. We will solder wires connecting the pads to SPST button switches.
Each has a pull-down resistor. GPIO 39 (center button) is also a control for
turning the board on and off. Push and hold for 3 seconds to turn on or off.

7) The board needs a uart, using the same provided on the HiLetGo ESP32 dev board
CP2102-GM. https://lcsc.com/product-detail/USB-ICs_SILICON-LABS-CP2102-GMR_C6568.html
USB connector at 180 degrees from ESP32 antennae for layout. Use Micro-USB C
to allow for battery charging during operation.

8) GPS module uses AT6558 running 3.3 volts. Breadboard shows connected to 5 volts
due to a requirement of the GPS breakout board. The breadboard uses the GPS + BDS
BeiDou Dual Module Flight Control Wiki - DFRobot breakout board. The board uses
only the AT6558 chip. And the board needs to provide an IPEX/uFl connector for the GPS antenna.
When I put the board into a wrist watch enclosure the GPS antenna will go into the wrist band.
https://www.digikey.com/en/products/detail/suntsu-electronics,-inc./SCNRF-6AAN-P1FS/14291136?utm_adgroup=Suntsu%20Electronics%20Inc&utm_source=google&utm_medium=cpc&utm_campaign=Shopping_DK%2BSupplier_Other&utm_term=&utm_content=Suntsu%20Electronics%20Inc&gclid=CjwKCAiAxJSPBhAoEiwAeO_fP0jt6Ce-0s7Bdl8Msl-FxEtlkEP35RCr5qoYZS6azBifDDCklZcshhoCEmIQAvD_BwE

9) Gesture sensor (VL53L5CX) is not mounted on the board, it is connected via
4 wires (VCC, Gnd, SCL, SDA/NAND), board needs 4 pads for soldering the wires.
https://www.st.com/resource/en/datasheet/vl53l5cx.pdf

10) Display 13-pin screen connector cable uses FPC connector. 1.28 inch lcd round is
a 240x240 display using GC9A01 driver HD ips.
https://www.aliexpress.com/item/1005002389910393.html
https://jlcpcb.com/parts/componentSearch?isSearch=true&searchTxt=XUNPU%20FPC-05F-12PH20
Also listed here:
https://pan.baidu.com/s/1x9B9jKrjikSCBUI38ZUC3w Extraction code: 8888
https://www.taobao.com/list/item/606659413574.htm?spm=a21wu.12321156.recommend-tpp.1

11) Breadboard uses an ESP32 WROOM Development Board. To minimize space Requirements
the board may use the ESP32-S3-MINI-1.
https://www.espressif.com/en/products/modules

12) Not all of the GPIO pins on the ESP32 are available to use.
See the Sox wiring guide for details:
https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Sox/Sox%20wiring%20guide.pdf
Do not attach to the GPIOs marked in red.
This is based on research from Andreas Speiss, see
https://www.youtube.com/watch?v=LY-1DHTxRAk and
https://drive.google.com/file/d/1gbKM7DA7PI7s1-ne_VomcjOrb0bE2TPZ/view

13) PCB design best practices: decoupling capacitors, landing patterns large enough
for the footprint, avoid too many via holes, avoid excessive trace lengths (especially
in the SPI bus between TFT, SD/NAND, and ESP32), avoid EMI from the ESP32 antennae, and
expect board revisions to correct unforseen problems.

14) Uses ESP32 processors with pre-burned ESP-IDF V3.1 bootloader. The boards must
be verified by connecting a USB cable to a laptop running Arduino IDE 1.8 or later
and flash a testing sketch.
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/bootloader.html

15) The board is not user expandable to add more devices to the I2C, SPI buses.

Changes to above:
Jan 19, 2022 - Mohammed using IMU LSM6DS3TR
https://lcsc.com/product-detail/Attitude-Sensors_STMicroelectronics-LSM6DS3TR-C_C967633.html
Jan 19, 2022 - Avinadad using AT6558R GPS (different from AT6558R, same pin layout)
Jan 19, 2022 - Avinadad ads GPIO 3 for battery voltage sensing
Jan 20, 2022 - Mohammed using 1 GB NAND/SD
https://lcsc.com/product-detail/NAND-FLASH_XTX-XTSD01GLGEAG_C558837.html
Jan 20, 2022 - Avinadad is basing higher pull-up resistors for I2C to lower the
battery drain. The board is not user expandable to add more devices to the I2C bus.
The board needs to operate while USB is connected and charging the battery.
I anticipate users plugging in a USB powerbank to keep the watch going as
they play games on it. To support this requires USB-C.
Jan 21, 2022 - Mohammed did not find a .7 pitch connector for the display.
I added the display datasheet and pointed him to:
https://www.aliexpress.com/item/1005002389910393.html
Jan 21, 2022 - Avinadad asked if the board needs a volume button. The board
will control volume through software only.
Jan 22, 2022 - Changed IMU and magnetometer/compass to these:
LIS2DH12 - 3 axis MEMS accelerometer
https://www.sparkfun.com/products/15760
https://jlcpcb.com/parts/componentSearch?isSearch=true&searchTxt=LIS2DH12
And,
Magnetometer - MMC5603NJ
https://datasheet.lcsc.com/lcsc/1912111437_MEMSIC-MMC5603NJ_C404328.pdf
Jan 22, 2022 - Changing from APDS-9960 gesture sensor to VL53L5CX sensor.
https://www.st.com/resource/en/datasheet/vl53l5cx.pdf
Jan 23, 2022 - Display cable 13 pin connector welded to the board,
not the adaptor board 7 pin connector
Jan 24, 2022 - Not all of the GPIO pins on the ESP32 are available to use.
See the Sox wiring guide for details:
https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Sox/Sox%20wiring%20guide.pdf
Do not attach to the GPIOs marked in red.
This is based on research from Andreas Speiss, see
https://www.youtube.com/watch?v=LY-1DHTxRAk and
https://drive.google.com/file/d/1gbKM7DA7PI7s1-ne_VomcjOrb0bE2TPZ/view
Jan 24, 2022 - Changing to USB C and adding a uart removes the need
for 'FTDI pads for pogo pins to flash the ESP32 bootloader'.
Jan 24, 2022 - Using this connector for the display adaptor. XUNPU FPC-05F-12PH20
https://jlcpcb.com/parts/componentSearch?isSearch=true&searchTxt=XUNPU%20FPC-05F-12PH20

Question? Ask principal maintainer Frank Cohen, fcohen@votsh.com
