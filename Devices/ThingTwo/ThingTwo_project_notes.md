ThingTwo project notes, Reflections

Hoober is the revision 2 of the Reflections main board. Hoober
fixes problems in the Sox board and gets closer to manufacturing.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, ready for manufacturing

Problems from the Hoober board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Hoober

What went wrong in Hoober:

- TFT display module pads used wrong pitch, the amazing Ryan Orosz of
Surface Mount Solutions in Santa Clara, California created a patch cable.

- Power-up did not work as expected, and was unneeded.

- GPS did not work, no satellites captured.

- NAND pads reversed.

- Feedback from the Element14 community says we should have no components,
  traces, and vias under and around the ESP32 antenna array, https://bit.ly/3Od68kH

- Power to the display cable did not work.

- Using Adafruit Feather ESP32-S3-Mini board definition. This appears
  incompatible with Platform.IO and the JTAG debugger support.

- ESP32-S3 does not reset after upload from Arduino IDE and esptool.py v3.3.1.
  This is a bug in the ESP32-S3 firmware.
  https://github.com/espressif/arduino-esp32/issues/6762

- Adafruit_DRV2605 software library does not support custom SPI pins for
  the Haptic feedback motor.

- Forgot to order PCB SMT stencils from PCBway. Had new ones produced
  in California by Surface Mount Solutions.

- Original PCBway order forgot to panelize the boards. Surface Mount Solutions
  of Santa Clara, California did the board assembly. They really like v-score
  instead of drilled tabs. And "north south east and west" orientation.

- Last minute parts swaps due to out-of-stock sitatuions.  

- Pandemic forced Shanghai city quarantine, slowing parts availability.

THINGTWO REQUIREMENTS
---------------------

ThingTwo is the 3rd revision of the Reflections main board. It is the successor to Hoober and Sox.

ThingTwo board changes:

+ Pickup Hoober rework changes, details in https://bit.ly/3PmWc9c

+ Remove power-on/off circuit and power button.

+ Replace GPS circuit with ATGM336H module. Power on/off control using a GPIO.

+ Display connector pads have corrected pitch.

+ Remove traces and vias around ESP32 antenna, https://bit.ly/3Od68kH

+ Add more room around the daughter board cable pads for easier soldering.

+ Label ESP32-S3-Mini-1-CUSTOM since we fixed some issues the Hoober footprint
  had with the Thermal Pads causing false flag DRC errors. It is the same as
  the previous footprints.

+ Put ESP32-S3 into JTAG USB mode on boot (GPIO3 is high on reset) AND make it
  possible to desolder a resistor (R19) to change GPIO3 to be low on reset for
  GPIO39-42 JTAG interface.


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

After the upload completes, I need to re-choose the USB Port, then click the Reset button.
I see the output correctly in the Serial Monitor.

Question? Ask principal maintainer Frank Cohen, fcohen@starlingwatch.com
