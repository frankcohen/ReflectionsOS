Horton project notes, Reflections

Started: January 23, 2023
Updated: Febrruary 7, 2023

Horton is the revision 4 of the Reflections main board. Horton
fixes problems in the ThingTwo board and gets the project into
software development.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, fixes problems
Horton - rev 4, ready for software development

Find the ThingTwo board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/ThingTwo

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

What went wrong in ThingTwo:
  NAND did not work because of the R22 pull up resistor caused the chip select not to work

  Gesture sensor daughter board did not work because I2C R18 and R19 resisters were
  not strong enough
  
  Switches B1 and B2 not installed, BOM specified switches that will not fit the board.
  
  Layout for the magnetometer U11 was wrong
  
HORTON REQUIREMENTS
---------------------

Horton board changes:

Remove R22 Pull Up resistor on the NAND CS, not needed
Remove R31 main board, it does nothing.
Magnetometer U11 now U5 uses the correct pad layout
Specify switches B1 and B2 that fit board layout
Update silk: Starling Watch 2023, Horton AM MAG
Remove J1, change J2 to DF65-6P use pin 3, 4 for the battery
Remove OPT3002DNPT from BOM
TPS2115APW power multiplexer is out of stock, replaced with 2 FDN340P MOSFETs Q1 Q2
Change TOF name to Gesture_Power and use GPIO 26 (Don't use 44 RXD0)
Replace LIS331DLHTR accelerometer with less expensive option
Add diode to USB/Battery power circuit
Some resistors and capacitors are out of stock, replace them

Stencil requirements for main board:

Board type :Panel by PCBWay
Panel Way :Panel in 1*2, total 5 sets=10pcs boards.
Different Design
in Panel：1
X-out Allowance in Panel :Accept
Size :86.69 x 47.78 mm
Quantity :5
Layer :
4 Layers
Material :FR-4: TG150
Thickness :
1.6 mm
Min Track/Spacing :6/6mil
Min Hole Size :
0.3mm ↑
Solder Mask :Green
Silkscreen :
White
Edge connector :No
Surface Finish :
HASL with lead
"HASL" to "ENIG"No
Via Process :
Tenting vias
Finished Copper :1 oz Cu (Inner Copper:1 oz)

Stencil for gesture daughter board:

Board type :Panel by PCBWay
Panel Way :Panel in 1*2, total 5 sets=10pcs boards.
Different Design
in Panel：1
X-out Allowance in Panel :Accept
Size :8 x 25.4 mm
Quantity :5
Layer :
2 Layers
Material :FR-4: TG150
Thickness :
1.6 mm
Min Track/Spacing :6/6mil
Min Hole Size :
0.2mm ↑
Solder Mask :Green
Silkscreen :
White
Edge connector :No
Surface Finish :
HASL with lead
"HASL" to "ENIG"No
Via Process :
Tenting vias
Finished Copper :1 oz Cu

NOTES
-----

Horton ESP32 boots in 'programmer' mode, ready to upload new firmware

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

After the upload completes, press the RESET button or power cycle the board

Testing Horton:

Untested:
Battery charging
JTAG debugging

Test sketches:
NAND, sketch 01SDFiles.ino, works
Display, GC9A01, 02DisplayTest.ino, backlight ok
Gestures, 03Gestures.ino, SparkFun_VL53L5CX_Library, works
GPS, ATGM336H-5N31, 04GPS.ino, works
Accelerometer, LIS331DLHTR, 05Accellerometer, uses Adafruit LIS331 library, works
Haptic feedback, 06Haptic.ino, Adafruit_DRV2605 library, works
Audio, 07AudioHomer.ino, ESP8266Audio  library, works
Magnetometer, 08Compass.ino, MMC5603NJ, chip id 4294967295,
  https://github.com/adafruit/Adafruit_MMC56x3, works

I2C devices
I2C device found at address 0x18 (24)  LIS331_DEFAULT_ADDRESS - accelerometer
I2C device found at address 0x29 (41)  Gesture sensor
I2C device found at address 0x30 (48)  Magnetometer, compass
I2C device found at address 0x5A (90)  Haptic controller

Things to-do list

Set-up Platform.io with ESP32-S3 board definition
One sketch to test all the components
Support Surface Mount Solutions (SMS) of Santa Clara, California to assemble Horton boards
Test display brightness using PWM
Test battery charging

Burning Man light sticks prototype
  Power from battery inside 1.5" PVC with WS2812 LEDs on outside
  
Calliope wrist watch prototype
  Test Horton board
  DONE Test brighter displays
  New case and strap
    Haptic motor, GPS antenna, battery, speaker
	How long does the battery last?
	Terri demonstration and approval
	

fcohen@starlingwatch.com March 4, 2023:
JLCPCB confirms their system is not properly showing inventory
for U4 MD7671A33PA1. They cancelled the order for that part:
https://jlcpcb.com/partdetail/998876-MD7671A33PA1/C920513
We found a component with the same footprint and function
at Mouser on https://bit.ly/3II03Mk. This is a Linear Regulator (LDO)
that puts out 3.3 volts from either USB 5V or battery 3.7 V input

Gesture sensor algorithm
Record or sensing

Record
  Start the sensor
  sensorHasSomething()
    recordGesture()
	  for 5 seconds, 4 frames per second
	    recordFrame()
	    until !sensortHasSomething()
	saveGesture()
	
Sensing
  Start the sensor
  sensorHasSomething()
    somethingMatchesFrame()
	  while moreFrames()
	    advanceToNextFrame()
      moreFrames()
		no, you found a match
	  keep history of matches and misses
		





