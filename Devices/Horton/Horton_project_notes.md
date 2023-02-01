Horton project notes, Reflections

January 23, 2023

Horton is the revision 4 of the Reflections main board. Horton
fixes problems in the ThingTwo board and gets closer to manufacturing.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, fixes problems
Horton - rev 4, ready for manufacturing

Find the ThingTwo board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/ThingTwo

Find the ThingTwo board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

What went wrong in ThingTwo:
  NAND did not work because of the R22 pull up resistor is not needed

  Gesture sensor daughter board did not work because I2C R18 and R19 resisters were
  not strong enough.
  
  Switches B1 and B2 not installed, BOM specified switches that will not fit the board.
  
  Layout for the magnetometer U11 was wrong
  
HORTON REQUIREMENTS
---------------------

Horton board changes:

  Remove R22 Pull Up resistor on the NAND CS, not needed
  Remove R31 main board, it does nothing.
  Magnetometer U11 uses the correct pad layout
  Specify switches B1 and B2 that fit board layout
  Update silk: Starling Watch 2023, Horton AW MAG
  Remove J1, change J2 to DF65-6P use pin 3, 4 for the battery
  Replace LIS331DLHTR accelerometer with less expensive option
  Add diode to USB/Battery power circuit
  Remove OPT3002DNPT from BOM
  Some resistors and capacitors are out of stock, replace them
  TPS2115APW power multiplexer is out of stock, replace with older less expensive multiplexer
  Change TOF name to Gesture_Power and use GPIO 26 (Don't use 44 RXD0)
  