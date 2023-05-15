Reflections Air Guitar on Horton board
Plays audio as mainboard is moved up-and-down as an "air guitar"
Plays .mp3 audio files over Bluetooth A2DP to portable speakers

May 13, 2023
Fun and game demonstration of Reflections Horton main board

Depends on
Arduino-Audio-Tools library 
https://github.com/pschatzmann/arduino-audio-tools
ESP32-A2DP library
https://github.com/pschatzmann/ESP32-A2DP
and
https://github.com/pschatzmann/arduino-libhelix

Uses example code from
https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-basic-api/base-player-a2dp/base-player-a2dp.ino
https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-sdfat-a2dp/player-sdfat-a2dp.ino
https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-sd-i2s/player-sd-i2s.ino

What this does:
player-sd-a2dp

Audio file processing needed:
Open sound file in Audacity. Make sure that it contains 2 channels
- Select Tracks -> Resample and select 44100
- Export -> Export Audio -> Header Raw ; Signed 16 bit PCM

ESP32 WROOM Dev Board
Flash size: 4 MB

Pins
17 - SD NAND CS
18 - SPI Clock
19 - SPI MISO
23 - SPI MOSI

22 - SCL
21 - SDA

Todo
Sketch to play music
Sense accelerometer being shaken, none = pause

