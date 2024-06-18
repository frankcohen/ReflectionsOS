Important patch to Adafruit_MMC56x3 library
fcohen@starlingwatch.com
June 14, 2024

Depends on a patched version of Adafruit_MMC56x3 1.0.1 library https://github.com/adafruit/Adafruit_MMC56x3

Some MMC5603 variants use (or ignore) the device ID code from WHO_AM_I register. For example, 
MMC5603NJ (https://jlcpcb.com/parts/componentSearch?searchTxt=MMC5603NJ&_t=1718386987988) returns 
an ID value of 0x00. 

I modified the library to have an overloaded begin() method accepting a custom ID value.

if (!mag.begin( MMC5603NJ_ADDRESS, &Wire, **MMC5603NJ_ID** ) ) 

Here is the patch to Adafruit_MMC56x3.h

bool begin(uint8_t i2c_address, TwoWire *wire, uint16_t chip_id );
uint16_t _sensorChipID = MMC56X3_CHIP_ID;

and the patch to Adafruit_MMC56x3.cpp

bool Adafruit_MMC5603::begin(uint8_t i2c_address, TwoWire *wire, uint16_t chip_id ) {
_sensorChipID = chip_id;
return begin( i2c_address, wire );
}

and

// make sure we're talking to the right chip
if (chip_id.read() != _sensorChipID) {
// No MMC56X3 detected ... return false
return false;
}

The patched library is in
libraries/Adafruit_MMC56x3_patched

I offered to patch the library in this issue:
https://github.com/adafruit/Adafruit_MMC56x3/issues/3

-Frank
