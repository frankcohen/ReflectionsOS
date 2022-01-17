# Adafruit VCNL4010 Library[![Build Status](https://github.com/adafruit/Adafruit_VCNL4010/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_VCNL4010/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_VCNL4010/html/index.html)

This is the Adafruit VCNL4010 Proximity I2C sensor library

Tested and works great with the Aadafruit VCNL4010 Breakout Board
  * http://www.adafruit.com/products/466

This chip uses I2C to communicate, 2 pins are required to interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Kevin Townsend/Limor Fried for Adafruit Industries.  
BSD license, check license.txt for more information
All text above must be included in any redistribution

To install, use the Arduino Library Manager to search for 'Adafruit VCNL4010'
and install the library.

<!-- START COMPATIBILITY TABLE -->

## Compatibility

MCU                | Tested Works | Doesn't Work | Not Tested  | Notes
------------------ | :----------: | :----------: | :---------: | -----
Atmega328 @ 16MHz  |      X       |             |            |
Atmega328 @ 12MHz  |      X       |             |            |
Atmega32u4 @ 16MHz |      X       |             |            | Use SDA/SCL on pins D2 &amp; D3
Atmega32u4 @ 8MHz  |      X       |             |            | Use SDA/SCL on pins D2 &amp; D3
ESP8266            |      X       |             |            | SDA/SCL default to pins 4 &amp; 5 but any two pins can be assigned as SDA/SCL using Wire.begin(SDA,SCL)
Atmega2560 @ 16MHz |      X       |             |            | Use SDA/SCL on pins 20 &amp; 21
ATSAM3X8E          |      X       |             |            | Use programming port.
ATSAM21D           |      X       |             |            | Use SDA and SCL pins.
ATtiny85 @ 16MHz   |      X       |             |            |
ATtiny85 @ 8MHz    |      X       |             |            |
Intel Curie @ 32MHz |             |             |     X       |
STM32F2            |             |             |     X       |

  * ATmega328 @ 16MHz : Arduino UNO, Adafruit Pro Trinket 5V, Adafruit Metro 328, Adafruit Metro Mini
  * ATmega328 @ 12MHz : Adafruit Pro Trinket 3V
  * ATmega32u4 @ 16MHz : Arduino Leonardo, Arduino Micro, Arduino Yun, Teensy 2.0
  * ATmega32u4 @ 8MHz : Adafruit Flora, Bluefruit Micro
  * ESP8266 : Adafruit Huzzah
  * ATmega2560 @ 16MHz : Arduino Mega
  * ATSAM3X8E : Arduino Due
  * ATSAM21D : Arduino Zero, M0 Pro
  * ATtiny85 @ 16MHz : Adafruit Trinket 5V
  * ATtiny85 @ 8MHz : Adafruit Gemma, Arduino Gemma, Adafruit Trinket 3V

<!-- END COMPATIBILITY TABLE -->
