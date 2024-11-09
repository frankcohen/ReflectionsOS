fcohen@starlingwatch.com, November 9, 2024

Reflections uses a custom partition scheme to maximize available memory.
On MacOS edit this file:
~/Library/Arduino15/packages/esp32/hardware/esp32/3.0.7/tools/partitions/boards.txt
add this:
esp32s3.menu.PartitionScheme.reflections=Reflections App (8MB OTA No SPIFFS)
esp32s3.menu.PartitionScheme.reflections.build.partitions=reflections
esp32s3.menu.PartitionScheme.reflections.upload.maximum_size=3342336   
create this file with these contents:
~/Library/Arduino15/packages/esp32/hardware/esp32/3.0.7/tools/partitions/reflections.csv
# Name, Type, SubType, Offset, Size, Flags
nvs,data,nvs,0x9000,20K,
otadata,data,ota,0xe000,8K,
ota_0,app,ota_0,0x10000,3712K,
ota_1,app,ota_1,0x3b0000,3712K,
eeprom,data,153,0x750000,4K,
spiffs,data,spiffs,0x751000,128K,
coredump,data,coredump,0x771000,64K,
Exit Arduino IDE
Delete ~/Library/Application\ Support/arduino-ide 
Start Arduino IDE
In the Select Board drop-down menu, select ESP32S3 Dev Module
In the Tools menu, select Partition Scheme, then Reflections

Arduino IDE does not input binary encoded partitions. ESP board
definition includes a gen_esp32part.py utility for setting the
correct memory range boundaries. Input is a CSV file with empty 
Offset values. Here is the input for the reflections partition table:

# Name,   Type, SubType, Offset,   Size,       Flags
nvs,      data, nvs,     0x9000,   0x5000,
otadata,  data, ota,     0xe000,   0x2000,
ota_0,    app,  ota_0,   ,         0x3A0000,
ota_1,    app,  ota_1,   ,         0x3A0000,
eeprom,   data, 0x99,    , 		   0x1000
spiffs,   data, spiffs,  ,         0x20000,
coredump, data, coredump,,         0x10000,

Use this command on MacOS Sonoma 14.6.1 (23G93) using Arduino IDE 2.3.3 and ESP32 board definition 3.0,7
python /Users/frankcohen/Downloads/espressif/esp32/tools/gen_esp32part.py --flash-size 8MB /Users/frankcohen/Documents/Arduino/ReflectionsOS/Devices/VladPlus/partitions/reflections-input.csv /Users/frankcohen/Documents/Arduino/ReflectionsOS/Devices/VladPlus/partitions/reflections.bin
The command creates a binary file.

The use this command
python /Users/frankcohen/Downloads/espressif/esp32/tools/gen_esp32part.py --flash-size 8MB /Users/frankcohen/Documents/Arduino/ReflectionsOS/Devices/VladPlus/partitions/reflections.bin /Users/frankcohen/Documents/Arduino/ReflectionsOS/Devices/VladPlus/partitions/reflections.csv
