# ReflectionsOS
ESP32 and Arduino-based Internet enabled watch project and reference architecture to make your own projects

![Reflections Mobile Experiences](Reflections_logo.jpg)

Many times I imagine making a mobile application connected to the Internet. For example, a wrist watch that shows videos of my children growing-up. I love the Arduino community and yet when I go to put something like this together I find many tutorials on the parts (like a display, Bluetooth, SD card, sound, and compass) there's very little I found that shows them integrated together. I am making my watch project and will be posting the code, datasheets, and design in the Reflections repository.

I love working with a group of creative makers. Please feel free to let me know your feedback and ideas. Thanks, in advance!

CindyLou is the latest version of the main board. CindyLou lets me put everything into a wrist watch case and actually wear it on my wrist! Plus the software to drive experiences (codename CalliopeOpensEyes) is coming along nicely. See https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments for experiments with the main board.

CindyLou Gerber files and schematic are at:
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou
EasyEDA project for the previous main board project is at:
https://oshwlab.com/fcohen/horton-main-board

I published articles on the technology needed to make Reflections:

JCUSB for JTAG debugging in Arduino IDE 2.1
https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/JCUSB%20using%20JTAG%2C%20CDC%2C%20USB%20for%20debugging.md

https://www.reddit.com/r/arduino/comments/mjygkl/using_tar_files_in_esp32_with_sd_applications_for

https://www.reddit.com/r/arduino/comments/mx7x31/file_transfer_using_bluetooth_classic_esp32_sd

https://www.reddit.com/r/arduino/comments/mbdj53/json_procedural_scripting_using_arduinojson_and

I used the EleksTube IPS clock for experimentation. It is an ESP32 device featuring 6 TFT displays. See
https://github.com/frankcohen/EleksTubeIPSHack

and I made an air-guitar project to learn about streaming music over Bluetooth Clasic, and why it won't work yet on BLE: https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/AirGuitar

-Frank

## Update as of July 18, 2023

A friend asked me: What can the Reflections board do? Consider these possibilities.

- Haptic feedback - a buzzer in the case. Buzzes softly or energetically. Fades the buzzing up, or down, and make patterns. It even does morse code - buzz buzz buzz.

- Long arm movement sensing - Recognizes the direction, speed, and acceleration of your wrist. Recognizes gestures made by moving your wrist in the air. Like sweeping left and right, slow pitching a ball.

- Finger movement sensing - recognizes gestures made with fingers.

- Range, Distance, Heading - when GPS is available, knows its position and direction/heading to other boards. When GPS is not available, such as being indoors, uses a compass and triangulation to detect movement towards or away from other boards using BLE.

- Compass - knows North from South and between.

- Location - when GPS is available, knows when it is in a location. For example, knows when it is at Disneyland and in line for the Matterhorn.

- Bump - touch another board and exchange data.

- USB flash memory - plug into your mobile phone or laptop to store and move files.

- Grab and play - capture video and audio of anything playing on your laptop, displays on the screen.

- [Martians](https://github.com/frankcohen/Martians) - yearn for each other, their hearts point to each other.

- Speaker to play sound and music. However, the speaker volume is very low. A future version needs an audio amplifier.

- Dance movements like pirouette

- Red light green light

- Head and shoulders, knees and toes

I'm glad to update this list with your ideas. Thanks.

##Update as of June 26, 2023:

CindyLou is the latest version of the main board. CindyLou lets me put everything into a wrist watch case and actually wear it on my wrist! Plus the software to drive experiences (codename CalliopeOpensEyes) is coming along nicely. See https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments for experiments with the main board.

![CindyLou version of the main board](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/CindyLou/CindyLou_main_board.jpg "CindyLou main board")

![Daughterboard for gesture sensing](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/CindyLou/CindyLou_daughter_board.jpg "CindyLou daughter board")

CindyLou Gerber files and schematic are at:
https://github.com/frankcohen/ReflectionsOS/tree/main/Devices/CindyLou
EasyEDA project for the previous main board project is at:
https://oshwlab.com/fcohen/horton-main-board


##Update as of April 1, 2023:

![I call this a Breadcard, it's not a breadboard but getting close to a wearable prototype, codenamed Horton](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Horton/HortonBreadcard.jpg "Horton Breadcard")

Update as of March 16, 2023:

Horton board is in from assembly and it works great! Next up, our first wearable prototype.
Expect many updates to the source code repository over the coming weeks and months.

##Update as of January 25, 2023:

All of the main board components are working. One more revision of the board is needed to incorporate the reworks we made to the ThingTwo board. We're calling the new revision Horton. It should be complete and go to manufacture and assembly in early February 2023.

The project team attention next goes to producing the first wearable prototype. This will strap to one's wrist, run on battery, and have a prototype display. We are also considering our first crowd funding campaign featuring the prototype. This is leading to an exciting Spring 2023.

##Update as of July 30, 2022:

We're making great progress on the main board, animation, and software projects. Main board revision 2 - codenamed Hoober - works and displays streaming video at 10 Frames Per Second (FPS). Revision 3 - codenamed ThingTwo - is designed and will go to manufacturing and assembly next week. See Devices/ThingTwo for design document, Gerber files, BOM, and wiring guide. Project focus will move to the software projects.

![Photo of the front side of the main board, codenamed Sox](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Hoober/Photos/Hoober_with_reworks_front.jpg "Main board front side")

![Photo of the back side of the main board, codenamed Sox](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Hoober/Photos/Hoober_with_reworks_back.jpg "Main board back side")

##Update as of April 27, 2022:

The project team is making progress on the main board and display. We received the first assembled main board in hand. It’s an ESP32 with GPS, IMU/compass, NAND memory, TFT display, audio output, USB, battery management, and finger gesture sensor, in a 34 mm round board. Cost for the prototype is $70 per board. This first version has about 3-4 problems that need reworking. The team is feeling good that the number of problems is less than 10. That puts us on track to have the finished board in Summer 2022. I will put the Gerber files and BOM into the repository for the upcoming revision 2 - code named Hoober. I am glad to send the Gerber files for your review, criticism, and feedback.

![Photo of the front side of the main board, codenamed Sox](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Sox/Photos/SoxAvinadadFront.jpg "Main board front side")

![Photo of the back side of the main board, codenamed Sox](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Sox/Photos/SoxAvinadadBack.jpg "Main board back side")
