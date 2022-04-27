# ReflectionsOS
ESP32 and Arduino-based Internet enabled watch project and reference architecture to make your own projects

Many times I imagine making a mobile application connected to the Internet. For example, a wrist watch. I love the Arduino community and yet when I go to put something like this together I find many tutorials on the parts (like a display, bluetooth, SD card, sound, and compass) there's very little I found that shows them all integrated together. I am making my watch project and will be posting the code, datasheets, and design in the Reflections repository.

I love working with a group of creative makers. Please feel free to let me know your feedback and ideas. Thanks, in advance!

The wrist watch is a way for me to enjoy home videos of my children on my wrist - my Reflections project. I published 3 articles on the technology needed to make Reflections:

https://www.reddit.com/r/arduino/comments/mjygkl/using_tar_files_in_esp32_with_sd_applications_for

https://www.reddit.com/r/arduino/comments/mx7x31/file_transfer_using_bluetooth_classic_esp32_sd

https://www.reddit.com/r/arduino/comments/mbdj53/json_procedural_scripting_using_arduinojson_and

I used the EleksTube IPS clock for experimentation. It is an ESP32 device featuring 6 TFT displays. See
https://github.com/frankcohen/EleksTubeIPSHack

-Frank

<hr>

Update as of April 27, 2022:

The project team is making progress on the main board and display. We received the first assembled main board in hand. It’s an ESP32 with GPS, IMU/compass, NAND memory, TFT display, audio output, USB, battery management, and finger gesture sensor, in a 34 mm round board. Cost for the prototype is $70 per board. This first version has about 3-4 problems that need reworking. The team is feeling good that the number of problems is less than 10. That puts us on track to have the finished board in Summer 2022. I will put the Gerber files and BOM into the repository for the upcoming revision 2 - code named Hoober. I am glad to send the Gerber files for your review, criticism, and feedback.

![Photo of the front side of the main board, codenamed Sox](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Sox/Photos/SoxAvinadadFront.jpg "Main board front side")

![Photo of the back side of the main board, codenamed Sox](https://github.com/frankcohen/ReflectionsOS/blob/main/Devices/Sox/Photos/SoxAvinadadBack.jpg "Main board back side")
