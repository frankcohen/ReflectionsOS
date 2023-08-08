BlueFish project notes, Reflections

Started: August 7, 2023

BlueFish is the 6th revision of the Reflections main board. 
It is the successor to CindyLou, Horton, ThingTwo, Hoober, Knox and Sox.
BlueFish replaces the LDO power circuit and the GPS unit.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, fixes problems
Horton - rev 4, ready for software development
CindyLou - rev 5, Beryl brighter display and component connectors

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

Find the CindyLou board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou

Find the BlueFish board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/BlueFish

What went wrong in CindyLou:

Power supply LDO (U4 MD7671A33PA1) fries when a battery is connected to the board. If the output from the LDO is higher than the input, current flows in reverse, destroying it. Horton used TPS2113APW as the power multiplexer. CindyLou kills LDOs, most likely due to a reverse polarity condition.

GPS unit (U14 ATGM336H-5N31) very rarely finds satellites consistently on start-up.

BLUEFISH REQUIREMENTS
---------------------

BlueFish board changes:

Change LDO to protect from reverse power conditions.

Update silk: Starling Watch 2023, BlueFish AM MAG

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
