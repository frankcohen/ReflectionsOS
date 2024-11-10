# ESP32-S3 partition tables and optimizing memory for Arduino IDE 2.x
## November 9, 2024, fcohen@starlingwatch.com

[Espressif](https://espressif.com/) tooling for ESP32 provides many options for managing memory, flash drives (SPIFFS), and Over The Air (OTA) firmware updating. My entry into ESP32 was through Arduino IDE's compatibility. I code in C/C++, debug over JTAG, and package updates for OTA delivery. I like Arduino IDE for it's simplicity, as compared to the powerful capabilities in Espress-IDF, PlatformIO, Intellij, Eclipse, and Visual Studio. Arduino IDE gets complicated to use when wanting to step beyond the default settings. For example, changing the partition tables for memory optimization takes a few steps.

My [Reflections project](https://github.com/frankcohen/ReflectionsOS) is a 34 mm round board for building entertaining mobile experiences. It is an ESP32-S3 with a bunch of sensors, storage, display, and speaker.

I created a custom ESP32 partition scheme to maximize available memory with 8 MB overall, 2 application spaces, OTA, and little or no SPIFFS file storage.

***Note: Members of the Arduino Forums pointed me to an easier solution: Simply name a custom partition file to partitions.csv, copy it into the sketch folder and select custom partition from the tools menu of the Arduino IDE.***

A custom partition file is a comma separated value (CSV) text file. Espressif documents the possible contents well [here](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32s3/api-guides/partition-tables.html). Here is my starting point:

```
# Name,   Type, SubType, Offset,   Size,       Flags
nvs,      data, nvs,     0x9000,   0x5000,
otadata,  data, ota,     0xe000,   0x2000,
ota_0,    app,  ota_0,   ,         0x3A0000,
ota_1,    app,  ota_1,   ,         0x3A0000,
eeprom,   data, 0x99,    , 		   0x1000
spiffs,   data, spiffs,  ,         0x20000,
coredump, data, coredump,,         0x10000,
```

Note I left the offset values empty. Offsets have their own pecular requirements. I found it easier to let the gen_esp32part.py utility provided by Espressif generate the offsets and the final partition file.

Use this command on MacOS Sonoma 14.6.1 (23G93) using Arduino IDE 2.3.3 and ESP32 board definition 3.0.7:

```
python ~/Downloads/espressif/esp32/tools/gen_esp32part.py --flash-size 8MB reflections-input.csv reflections.bin
```

The utility creates a binary file.

Arduino IDE does not input binary encoded partitions. The fun in gen_esp32part.py is using it in both directions. This command converts the binary file into a csv text file:

```
python ~/Downloads/espressif/esp32/tools/gen_esp32part.py --flash-size 8MB reflections-input.csv reflections.bin
```

reflections.csv contains this:

```
# Name, Type, SubType, Offset, Size, Flags
nvs,data,nvs,0x9000,20K,
otadata,data,ota,0xe000,8K,
ota_0,app,ota_0,0x10000,3712K,
ota_1,app,ota_1,0x3b0000,3712K,
eeprom,data,153,0x750000,4K,
spiffs,data,spiffs,0x751000,128K,
coredump,data,coredump,0x771000,64K,
```

Installing a partition to Arduino IDE is a manual effort. Copy the reflections.csv file on MacOS for ESP32-S3 Dev Module 3.0.7 to:

```
~/Library/Arduino15/packages/esp32/hardware/esp32/3.0.7/tools/partitions/reflections.csv
```

Then tell Arduino IDE 2.3.3 where to find the partition by editing this file:

```
~/Library/Arduino15/packages/esp32/hardware/esp32/3.0.7/tools/partitions/boards.txt
```

Reflections is compatible with the ESP32-S3 Dev Module board definition, so add this to boards.txt:

```
esp32s3.menu.PartitionScheme.reflections=Reflections App (8MB OTA No SPIFFS)
esp32s3.menu.PartitionScheme.reflections.build.partitions=reflections
esp32s3.menu.PartitionScheme.reflections.upload.maximum_size=3342336   
```

Next, if Arduino IDE is open, exit it. Then manually delete this directory:

```
~/Library/Application\ Support/arduino-ide
```

Start Arduino IDE

In the Select Board drop-down menu, select ESP32S3 Dev Module

In the Tools menu, select Partition Scheme, then select:

Reflections App (8MB OTA No SPIFFS)

The next time you compile and upload the ESP32-S3 will use the partition values. Set Tools -> Core Debug Level to Verbose and see the custom partition values used in the Serial Monitor as your app starts to run.

```
------------------------------------------
Partitions Info:
------------------------------------------
                nvs : addr: 0x00009000, size:    20.0 KB, type: DATA, subtype: NVS
            otadata : addr: 0x0000E000, size:     8.0 KB, type: DATA, subtype: OTA
              ota_0 : addr: 0x00010000, size:  3712.0 KB, type:  APP, subtype: OTA_0
              ota_1 : addr: 0x003B0000, size:  3712.0 KB, type:  APP, subtype: OTA_1
             eeprom : addr: 0x00750000, size:     4.0 KB, type: DATA, subtype: 0x99
             spiffs : addr: 0x00751000, size:   128.0 KB, type: DATA, subtype: SPIFFS
           coredump : addr: 0x00771000, size:    64.0 KB, type: DATA, subtype: COREDUMP
```
		   
Please let me know your experience. I want to make sure these instructions are accurate and wise. Thanks.

-Frank
