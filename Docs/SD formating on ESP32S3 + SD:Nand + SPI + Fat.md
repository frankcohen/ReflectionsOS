# ESP32-S3 SPI Nand/SD SdFat Format for Fat16 recipe

by Frank Cohen, November 2, 2023, part of the [Reflections](https://github.com/frankcohen/ReflectionsOS) open-source project

## A Recipe

SdFat is an amazing open-source library for Arduino and ESP32 projects to use SD and Nand storage. It's been around since 2009. @greiman does an amazing job, even if SdFat is impossible to maintain. This is my contribution, a recipe for fellow ESP32 enthusiasts needing to format an SD for Fat16 over SPI. I wrote it for my [Reflections](https://github.com/frankcohen/ReflectionsOS) open-source project. It uses an ESP32-S3 and Nand/SD.

### Install SdFat

I used SdFat version 2.2.2. Open the Library Manager in Arduino IDE 2.2.1.

### SdFatConfig.h Configuration

Make these changes to:

arduino/libraries/SdFat/src/SdFatConfig.h

Find this definition, change its value to 3

```
#define SPI_DRIVER_SELECT 3
```

### Schematic

<img src="https://raw.githubusercontent.com/frankcohen/ReflectionsOS/main/Docs/images/SD-ESP32-SdFat.jpg" alt="SD attaches to ESP32 over SPI" width="100%"/>

### Sketch

Refer to the code 
[https://github.com/frankcohen/ReflectionsOS/Experiments/SdFat_ESP32_SPI_SD_NAND_Formatting](https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/SdFat_ESP32_SPI_SD_NAND_Formatting)

GPIO 12 is the chip-select for a TFT video display on the same SPI bus as the SD/Nand. This tells SdFat to disable it.
```
const int8_t DISABLE_CS_PIN = 12;
```

GPIO 15 is the chip-select for the SD/Nand.
```
#define SD_CS_PIN 15
```

ESP32 allows any GPIO pins to be an SPI bus. See [Andreas Speiss video and spreadsheet](https://www.youtube.com/watch?v=LY-1DHTxRAk) to know the limitations. Reflection's board uses 35, 36, 37 for SPI between the SD/Nand and Display.
```
#define SPI_MOSI      35
#define SPI_MISO      37
#define SPI_SCK       36
```

Additional GPIO pins used by the Reflections board display.
```
#define Display_SPI_DC    5
#define Display_SPI_CS    12
#define Display_SPI_RST   0
#define Display_SPI_BK    6
```

Custom SPI connection class, begin method starts the SPI bus using the GPIO pins.
```
class MySpiClass : public SdSpiBaseClass {
...
SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);
...
```

SdFat sets SPI bus speed with the SD_SCK_MHZ macro. 50 works for my ESP32-S3-Mini-N1. I have found no clear method of determining the speed for a given board and SD/Nand component. Try lesser values. For example, 30, 20, 10.
```
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50), &mySpi)
```

Sets the GPIO pins that support the SD/Nand and the display. These are specific to the [Reflections](https://github.com/frankcohen/ReflectionsOS) board and you may change them for your board.
```
  pinMode(SD_CS_PIN, OUTPUT );
  digitalWrite(SD_CS_PIN, LOW);

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, LOW);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);
```

## Arduino IDE 2.x settings

Using Arduino IDE 2.x, use Tools menu options:

```
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
Core Debug Level: Error
USB DFU On Boot: Disabled
Erase All Flash Before Sketch Upload: Disabled
Events Run On: Core 1
Flash Mode: QIO 80 Mhz
Flash Size: 8 MB (64MB)
JTAG Adapter: Integrated USB JTAG
Arduino Runs On: Core 1
USB Firmware MSC On Boot: Disabled
Partition Scheme: Defaults 4MB with Spiffs (1.2 MB APP/1.5MB SPIFFS)
PSRAM: Disabled
Upload Mode: UART0/Hardware CDC
Upload Speed 921600
USB Mode: Hardware CDC and JTAG
```

## JCUSB, USB, JTAG Debugging, Connection

Connect your computer to an ESP32-S3 using a technique I call [JCUSB](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/JCUSB%20using%20JTAG%2C%20CDC%2C%20USB%20for%20debugging.md). JCUSB is short for S3 + USB + CDC + OpenOCD + Arduino IDE. Only ESP32-S3 and its successors support JCUSB - it won't work on ESP32 and ESP32-S2. JCUSB uses a custom USB cable to connect an ESP32 to a Windows or MacOS computer. [Details here](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/JCUSB%20using%20JTAG%2C%20CDC%2C%20USB%20for%20debugging.md)

# Compile and Run

Once you customize the above values, compile and upload the code. Then open the Serial Monitor, set the connection speeed to 115200, follow the on-screen instructions.

Here is an example. I ran this on my MacOS laptop using Arduino IDE 2.2.1.

```
SdFat version: 2.2.2

Disabling SPI device on pin 12

Assuming the SD chip select pin is: 15
Edit SD_CS_PIN to change the SD chip select pin.
Done

type any character to start
```

## What May Go Wrong

SdFat works with a lot of hardware and SD cards, and it is possible it will not work with yours.

- SPI bus speed. SdFat sets SPI bus speed with the SD_SCK_MHZ macro. 50 works for my ESP32-S3-Mini-N1. I have found no clear method of determining the speed for a given board and SD/Nand component. Try lesser values. For example, 30, 20, 10.

- SD card won't format. Usually because it is broken. Or maybe partially zapped from static electricity. Or it is a low quality SD card that works inconsistently.

- Conflicts with the board definitions. Check the pin use on your board.

In the event something goes wrong, SdFat's creator/maintainer does an amazing job at interacting with his community. I recommend you search the [issues](https://github.com/greiman/SdFat/issues), and if you need to post an issue.

--

Frank Cohen, 2023
[Reflections Project](https://github.com/frankcohen/ReflectionsOS)

