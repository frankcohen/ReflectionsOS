CindyLou project notes, Reflections

Started: February 21, 2023
Updated: February 21, 2023

February 21, 2023

CindyLou is the 5th revision of the Reflections main board. 
It is the successor to Horton, ThingTwo, Hoober, Knox and Sox.
CindyLou replaces the previous display for something brighter.

Sox - 1st generation board design
Hoober - rev 2, fixes problems
ThingTwo - rev 3, fixes problems
Horton - rev 4, ready for software development

Find the Horton board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/Horton

Find the CindyLou board project
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou

What went wrong in Horton:




CINDYLOU REQUIREMENTS
---------------------

CindyLou board changes:

Change the display pad layout to support
the Beryl Zeng CT013A6QQIGC7N10-XHT display
It does 750 NITS and appears good and bright

Update silk: Starling Watch 2023, CindyLou AM MAG

V-scoring, and the scoring needs to be against a flat (not round) part of the board

Daughter board should be the VLC53 chip on a flexible cable board, with the resisters/caps on the main board, so there’s no welding needed at the daughter board for the cable, 

USB connector is super tight against the board, needs a little tolerance





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
