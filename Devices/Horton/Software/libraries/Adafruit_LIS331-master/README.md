# Adafruit LIS331 [![Build Status](https://github.com/adafruit/Adafruit_LIS331/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_LIS331/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_LIS331/html/index.html)

<a href="https://www.adafruit.com/product/4626"><img src="" width="500px" /></a>

This is the Arduino library for the Adafruit LIS331 family of 3-axis Accelerometers.

## Supported Hardware
* [LIS331HH High-g 3-Axis Accelerometer](https://www.adafruit.com/products/4XXX)
* [H3LIS331 Very High-g 3-Axis Accelerometer](https://www.adafruit.com/products/4XXX)

This sensor communicates over I2C or SPI (our library code supports both) so you can share it with a bunch of other sensors on the same I2C bus.
There's an address selection pin so you can have two accelerometers share an I2C bus.

# Dependencies
 * [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO)
 * [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor)

# Contributing

Contributions are welcome! Please read our [Code of Conduct](https://github.com/adafruit/Adafruit_LIS331/blob/master/CODE_OF_CONDUCT.md>)
before contributing to help this project stay welcoming.
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
## Documentation and doxygen
Documentation is produced by doxygen. Contributions should include documentation for any new code added.

Some examples of how to use doxygen can be found in these guide pages:

https://learn.adafruit.com/the-well-automated-arduino-library/doxygen

https://learn.adafruit.com/the-well-automated-arduino-library/doxygen-tips

Written by Bryan Siepert / K. Townsend / Limor Fried for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution

To install, use the Arduino Library Manager and search for "Adafruit LIS331" and install the library.
