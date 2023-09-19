RedFish project notes, Reflections

Started: August 7, 2023

RedFish is the 6th revision of the Reflections main board. 
It is the successor to CindyLou, Horton, ThingTwo, Hoober, Knox and Sox.
RedFish replaces the LDO power circuit and the GPS unit.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, fixes problems
Horton - rev 4, ready for software development
CindyLou - rev 5, Beryl brighter display and component connectors

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

Find the CindyLou board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou

Find the RedFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/RedFish

What went wrong in CindyLou:

Power supply LDO (U4 MD7671A33PA1) and battery charging IC fried when a battery is connected to the board. If the output from the LDO is higher than the input, current flows in reverse, destroying it. Horton used TPS2113APW as the power multiplexer. CindyLou kills LDOs, most likely due to a reverse polarity condition.

Mostfet switch circuit was not correct for gesture power and TFT display power, causing them to be always on.

U1 for Haptic motor was difficult to wire because of its position on the board.

GPS unit (U14 ATGM336H-5N31) very rarely finds satellites consistently on start-up.

RedFish REQUIREMENTS
---------------------

RedFish board changes:

1. Change the LDO with AZ1117CR2-3.3TRG1
2. Change the charging IC with XC6802A42XMR-G
3. Change the mosfet switch circuit for gesture power
4. Change the mosfet switch circuit for TFT LCD
5. Move the Haptic connector 1 mm towards U1
6. Tidy up for the silk screen
7. Investigate the GPS start-up problem

Update silk: Starling Watch 2023, RedFish T4 AM MAG

T4 is for Thi Thu Thao Tran, a talented electronics engineer who made the changes for RedFish

Do not place GPIO header.

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
