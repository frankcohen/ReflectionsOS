/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#include "Arduino_SWSPI.h"

Arduino_SWSPI::Arduino_SWSPI(int8_t dc, int8_t cs, int8_t sck, int8_t mosi, int8_t miso /* = -1 */)
    : _dc(dc), _cs(cs), _sck(sck), _mosi(mosi), _miso(miso)
{
}

void Arduino_SWSPI::begin(int32_t speed, int8_t dataMode)
{
  UNUSED(speed);
  UNUSED(dataMode);

  if (_dc >= 0)
  {
    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH); // Data mode
  }
  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // Deselect
  }
  pinMode(_mosi, OUTPUT);
  digitalWrite(_mosi, LOW);
  pinMode(_sck, OUTPUT);
  digitalWrite(_sck, LOW);
  if (_miso >= 0)
  {
    pinMode(_miso, INPUT);
  }

#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(ARDUINO_ARCH_NRF52840)
  uint32_t pin = digitalPinToPinName((pin_size_t)_sck);
  NRF_GPIO_Type *reg = nrf_gpio_pin_port_decode(&pin);
  _sckPortSet = &reg->OUTSET;
  _sckPortClr = &reg->OUTCLR;
  _sckPinMask = 1UL << pin;
  pin = digitalPinToPinName((pin_size_t)_mosi);
  reg = nrf_gpio_pin_port_decode(&pin);
  nrf_gpio_cfg_output(pin);
  _mosiPortSet = &reg->OUTSET;
  _mosiPortClr = &reg->OUTCLR;
  _mosiPinMask = 1UL << pin;
  if (_dc >= 0)
  {
    pin = digitalPinToPinName((pin_size_t)_dc);
    reg = nrf_gpio_pin_port_decode(&pin);
    nrf_gpio_cfg_output(pin);
    _dcPortSet = &reg->OUTSET;
    _dcPortClr = &reg->OUTCLR;
    _dcPinMask = 1UL << pin;
  }
  if (_cs >= 0)
  {
    pin = digitalPinToPinName((pin_size_t)_cs);
    reg = nrf_gpio_pin_port_decode(&pin);
    nrf_gpio_cfg_output(pin);
    _csPortSet = &reg->OUTSET;
    _csPortClr = &reg->OUTCLR;
    _csPinMask = 1UL << pin;
  }
  if (_miso >= 0)
  {
    pin = digitalPinToPinName((pin_size_t)_cs);
    reg = nrf_gpio_pin_port_decode(&pin);
    _misoPort = &reg->IN;
    _misoPinMask = 1UL << pin;
  }
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  _sckPinMask = digitalPinToBitMask(_sck);
  _mosiPinMask = digitalPinToBitMask(_mosi);
  _sckPortSet = (PORTreg_t)&sio_hw->gpio_set;
  _sckPortClr = (PORTreg_t)&sio_hw->gpio_clr;
  _mosiPortSet = (PORTreg_t)&sio_hw->gpio_set;
  _mosiPortClr = (PORTreg_t)&sio_hw->gpio_clr;
  _dcPinMask = digitalPinToBitMask(_dc);
  _dcPortSet = (PORTreg_t)&sio_hw->gpio_set;
  _dcPortClr = (PORTreg_t)&sio_hw->gpio_clr;
  if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&sio_hw->gpio_set;
    _csPortClr = (PORTreg_t)&sio_hw->gpio_clr;
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = (PORTreg_t)_dcPortSet;
    _csPortClr = (PORTreg_t)_dcPortClr;
    _csPinMask = 0;
  }
  if (_miso >= 0)
  {
    _misoPort = portInputRegister(_miso);
    _misoPinMask = digitalPinToBitMask(_miso);
  }
  else
  {
    _misoPort = portInputRegister(_miso);
  }
  if (_miso >= 0)
  {
    _misoPinMask = digitalPinToBitMask(_miso);
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_miso));
  }
  else
  {
    _misoPinMask = 0;
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_sck));
  }
#elif CONFIG_IDF_TARGET_ESP32C3
  _sckPinMask = digitalPinToBitMask(_sck);
  _sckPortSet = (PORTreg_t)&GPIO.out_w1ts;
  _sckPortClr = (PORTreg_t)&GPIO.out_w1tc;
  _mosiPinMask = digitalPinToBitMask(_mosi);
  _mosiPortSet = (PORTreg_t)&GPIO.out_w1ts;
  _mosiPortClr = (PORTreg_t)&GPIO.out_w1tc;
  _dcPinMask = digitalPinToBitMask(_dc);
  _dcPortSet = (PORTreg_t)&GPIO.out_w1ts;
  _dcPortClr = (PORTreg_t)&GPIO.out_w1tc;
  if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _csPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = _dcPortSet;
    _csPortClr = _dcPortClr;
    _csPinMask = 0;
  }
  if (_miso >= 0)
  {
    _misoPinMask = digitalPinToBitMask(_miso);
    _misoPort = (PORTreg_t)GPIO_IN_REG;
  }
  else
  {
    _misoPinMask = 0;
    _misoPort = (PORTreg_t)GPIO_IN_REG;
  }
#elif defined(ESP32)
  _sckPinMask = digitalPinToBitMask(_sck);
  _mosiPinMask = digitalPinToBitMask(_mosi);
  if (_sck >= 32)
  {
    _sckPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _sckPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _sckPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _sckPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  if (_mosi >= 32)
  {
    _mosiPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _mosiPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _mosiPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _mosiPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  _dcPinMask = digitalPinToBitMask(_dc);
  if (_dc >= 32)
  {
    _dcPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _dcPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else
  {
    _dcPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _dcPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  if (_cs >= 32)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _csPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _csPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = (PORTreg_t)_dcPortSet;
    _csPortClr = (PORTreg_t)_dcPortClr;
    _csPinMask = 0;
  }
  if (_miso >= 0)
  {
    _misoPinMask = digitalPinToBitMask(_miso);
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_miso));
  }
  else
  {
    _misoPinMask = 0;
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_sck));
  }
#elif defined(CORE_TEENSY)
#if !defined(KINETISK)
  _sckPinMask = digitalPinToBitMask(_sck);
  _mosiPinMask = digitalPinToBitMask(_mosi);
#endif
  _sckPortSet = portSetRegister(_sck);
  _sckPortClr = portClearRegister(_sck);
  _mosiPortSet = portSetRegister(_mosi);
  _mosiPortClr = portClearRegister(_mosi);
  if (_dc >= 0)
  {
#if !defined(KINETISK)
    _dcPinMask = digitalPinToBitMask(_dc);
#endif
    _dcPortSet = portSetRegister(_dc);
    _dcPortClr = portClearRegister(_dc);
  }
  else
  {
#if !defined(KINETISK)
    _dcPinMask = 0;
#endif
    _dcPortSet = _sckPortSet;
    _dcPortClr = _sckPortClr;
  }
  if (_cs >= 0)
  {
#if !defined(KINETISK)
    _csPinMask = digitalPinToBitMask(_cs);
#endif
    _csPortSet = portSetRegister(_cs);
    _csPortClr = portClearRegister(_cs);
  }
  else
  {
#if !defined(KINETISK)
    _csPinMask = 0;
#endif
    _csPortSet = _sckPortSet;
    _csPortClr = _sckPortClr;
  }
#else  // !CORE_TEENSY
  _sckPinMask = digitalPinToBitMask(_sck);
  _mosiPinMask = digitalPinToBitMask(_mosi);
  _sckPortSet = &(PORT->Group[g_APinDescription[_sck].ulPort].OUTSET.reg);
  _sckPortClr = &(PORT->Group[g_APinDescription[_sck].ulPort].OUTCLR.reg);
  _mosiPortSet = &(PORT->Group[g_APinDescription[_mosi].ulPort].OUTSET.reg);
  _mosiPortClr = &(PORT->Group[g_APinDescription[_mosi].ulPort].OUTCLR.reg);
  if (_dc >= 0)
  {
    _dcPinMask = digitalPinToBitMask(_dc);
    _dcPortSet = &(PORT->Group[g_APinDescription[_dc].ulPort].OUTSET.reg);
    _dcPortClr = &(PORT->Group[g_APinDescription[_dc].ulPort].OUTCLR.reg);
  }
  else
  {
    // No D/C line defined; 9-bit SPI.
    // Assign a valid GPIO register (though not used for DC), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _dc and possibly branching.
    _dcPortSet = _sckPortSet;
    _dcPortClr = _sckPortClr;
    _dcPinMask = 0;
  }
  if (_cs >= 0)
  {
    _csPinMask = digitalPinToBitMask(_cs);
    _csPortSet = &(PORT->Group[g_APinDescription[_cs].ulPort].OUTSET.reg);
    _csPortClr = &(PORT->Group[g_APinDescription[_cs].ulPort].OUTCLR.reg);
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPortSet = _sckPortSet;
    _csPortClr = _sckPortClr;
    _csPinMask = 0;
  }
  if (_miso >= 0)
  {
    _misoPinMask = digitalPinToBitMask(_miso);
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_miso));
  }
  else
  {
    _misoPinMask = 0;
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_sck));
  }
#endif // end !CORE_TEENSY
#else  // !HAS_PORT_SET_CLR
  _sckPort = (PORTreg_t)portOutputRegister(digitalPinToPort(_sck));
  _sckPinMaskSet = digitalPinToBitMask(_sck);
  _mosiPort = (PORTreg_t)portOutputRegister(digitalPinToPort(_mosi));
  _mosiPinMaskSet = digitalPinToBitMask(_mosi);
  if (_dc >= 0)
  {
    _dcPort = (PORTreg_t)portOutputRegister(digitalPinToPort(_dc));
    _dcPinMaskSet = digitalPinToBitMask(_dc);
  }
  else
  {
    // No D/C line defined; 9-bit SPI.
    // Assign a valid GPIO register (though not used for DC), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _dc and possibly branching.
    _dcPort = _sckPort;
    _dcPinMaskSet = 0;
  }
  if (_cs >= 0)
  {
    _csPort = (PORTreg_t)portOutputRegister(digitalPinToPort(_cs));
    _csPinMaskSet = digitalPinToBitMask(_cs);
  }
  else
  {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    _csPort = _sckPort;
    _csPinMaskSet = 0;
  }
  if (_miso >= 0)
  {
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_miso));
    _misoPinMask = digitalPinToBitMask(_miso);
  }
  else
  {
    _misoPort = (PORTreg_t)portInputRegister(digitalPinToPort(_sck));
    _misoPinMask = 0;
  }
  _csPinMaskClr = ~_csPinMaskSet;
  _dcPinMaskClr = ~_dcPinMaskSet;
  _sckPinMaskClr = ~_sckPinMaskSet;
  _mosiPinMaskClr = ~_mosiPinMaskSet;
#endif // !HAS_PORT_SET_CLR
#endif // USE_FAST_PINIO
}

void Arduino_SWSPI::beginWrite()
{
  if (_dc >= 0)
  {
    DC_HIGH();
  }
  CS_LOW();
}

void Arduino_SWSPI::endWrite()
{
  CS_HIGH();
}

void Arduino_SWSPI::writeCommand(uint8_t c)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BITCOMMAND(c);
  }
  else
  {
    DC_LOW();
    WRITE(c);
    DC_HIGH();
  }
}

void Arduino_SWSPI::writeCommand16(uint16_t c)
{
  if (_dc < 0) // 9-bit SPI
  {
    _data16.value = c;
    WRITE9BITCOMMAND(_data16.msb);
    WRITE9BITCOMMAND(_data16.lsb);
  }
  else
  {
    DC_LOW();
    WRITE16(c);
    DC_HIGH();
  }
}

void Arduino_SWSPI::write(uint8_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BITDATA(d);
  }
  else
  {
    WRITE(d);
  }
}

void Arduino_SWSPI::write16(uint16_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    _data16.value = d;
    WRITE9BITDATA(_data16.msb);
    WRITE9BITDATA(_data16.lsb);
  }
  else
  {
    WRITE16(d);
  }
}

void Arduino_SWSPI::writeRepeat(uint16_t p, uint32_t len)
{
  if (_dc < 0) // 9-bit SPI
  {
// ESP8266 avoid trigger watchdog
#if defined(ESP8266)
    while (len > (ESP8266SAFEBATCHBITSIZE / 9))
    {
      WRITE9BITREPEAT(p, ESP8266SAFEBATCHBITSIZE / 9);
      len -= ESP8266SAFEBATCHBITSIZE / 9;
      yield();
    }
    WRITE9BITREPEAT(p, len);
#else
    WRITE9BITREPEAT(p, len);
#endif
  }
  else
  {
#if defined(ESP8266)
    while (len > (ESP8266SAFEBATCHBITSIZE / 8))
    {
      WRITEREPEAT(p, ESP8266SAFEBATCHBITSIZE / 8);
      len -= ESP8266SAFEBATCHBITSIZE / 8;
      yield();
    }
    WRITEREPEAT(p, len);
#else
    WRITEREPEAT(p, len);
#endif
  }
}

void Arduino_SWSPI::writePixels(uint16_t *data, uint32_t len)
{
  while (len--)
  {
    WRITE16(*data++);
  }
}

#if !defined(LITTLE_FOOT_PRINT)
void Arduino_SWSPI::writeBytes(uint8_t *data, uint32_t len)
{
  while (len--)
  {
    WRITE(*data++);
  }
}

void Arduino_SWSPI::writePattern(uint8_t *data, uint8_t len, uint32_t repeat)
{
  while (repeat--)
  {
    for (uint8_t i = 0; i < len; i++)
    {
      WRITE(data[i]);
    }
  }
}
#endif // !defined(LITTLE_FOOT_PRINT)

INLINE void Arduino_SWSPI::WRITE9BITCOMMAND(uint8_t c)
{
  // D/C bit, command
  SPI_MOSI_LOW();
  SPI_SCK_HIGH();
  SPI_SCK_LOW();

  uint8_t bit = 0x80;
  while (bit)
  {
    if (c & bit)
    {
      SPI_MOSI_HIGH();
    }
    else
    {
      SPI_MOSI_LOW();
    }
    SPI_SCK_HIGH();
    bit >>= 1;
    SPI_SCK_LOW();
  }
}

INLINE void Arduino_SWSPI::WRITE9BITDATA(uint8_t d)
{
  // D/C bit, data
  SPI_MOSI_HIGH();
  SPI_SCK_HIGH();
  SPI_SCK_LOW();

  uint8_t bit = 0x80;
  while (bit)
  {
    if (d & bit)
    {
      SPI_MOSI_HIGH();
    }
    else
    {
      SPI_MOSI_LOW();
    }
    SPI_SCK_HIGH();
    bit >>= 1;
    SPI_SCK_LOW();
  }
}

INLINE void Arduino_SWSPI::WRITE(uint8_t d)
{
  uint8_t bit = 0x80;
  while (bit)
  {
    if (d & bit)
    {
      SPI_MOSI_HIGH();
    }
    else
    {
      SPI_MOSI_LOW();
    }
    SPI_SCK_HIGH();
    bit >>= 1;
    SPI_SCK_LOW();
  }
}

INLINE void Arduino_SWSPI::WRITE16(uint16_t d)
{
  uint16_t bit = 0x8000;
  while (bit)
  {
    if (d & bit)
    {
      SPI_MOSI_HIGH();
    }
    else
    {
      SPI_MOSI_LOW();
    }
    SPI_SCK_HIGH();
    bit >>= 1;
    SPI_SCK_LOW();
  }
}

INLINE void Arduino_SWSPI::WRITE9BITREPEAT(uint16_t p, uint32_t len)
{
  if (p == 0xffff) // no need to set MOSI level while filling white
  {
    SPI_MOSI_HIGH();
    len *= 18; // 9-bit * 2
    while (len--)
    {
      SPI_SCK_HIGH();
      SPI_SCK_LOW();
    }
  }
  else
  {
    _data16.value = p;
    while (len--)
    {
      WRITE9BITDATA(_data16.msb);
      WRITE9BITDATA(_data16.lsb);
    }
  }
}

INLINE void Arduino_SWSPI::WRITEREPEAT(uint16_t p, uint32_t len)
{
  if ((p == 0x0000) || (p == 0xffff)) // no need to set MOSI level while filling black or white
  {
    if (p)
    {
      SPI_MOSI_HIGH();
    }
    else
    {
      SPI_MOSI_LOW();
    }
    len *= 16;
    while (len--)
    {
      SPI_SCK_HIGH();
      SPI_SCK_LOW();
    }
  }
  else
  {
    while (len--)
    {
      WRITE16(p);
    }
  }
}

/******** low level bit twiddling **********/

INLINE void Arduino_SWSPI::DC_HIGH(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_dcPortSet = 1;
#else  // !KINETISK
  *_dcPortSet = _dcPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
  *_dcPort |= _dcPinMaskSet;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_dc, HIGH);
#endif // end !USE_FAST_PINIO
}

INLINE void Arduino_SWSPI::DC_LOW(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_dcPortClr = 1;
#else  // !KINETISK
  *_dcPortClr = _dcPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
  *_dcPort &= _dcPinMaskClr;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_dc, LOW);
#endif // end !USE_FAST_PINIO
}

INLINE void Arduino_SWSPI::CS_HIGH(void)
{
  if (_cs >= 0)
  {
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
    *_csPortSet = 1;
#else  // !KINETISK
    *_csPortSet = _csPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
    *_csPort |= _csPinMaskSet;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
    digitalWrite(_cs, HIGH);
#endif // end !USE_FAST_PINIO
  }
}

INLINE void Arduino_SWSPI::CS_LOW(void)
{
  if (_cs >= 0)
  {
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
    *_csPortClr = 1;
#else  // !KINETISK
    *_csPortClr = _csPinMask;
#endif // end !KINETISK
#else  // !HAS_PORT_SET_CLR
    *_csPort &= _csPinMaskClr;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
    digitalWrite(_cs, LOW);
#endif // end !USE_FAST_PINIO
  }
}

/*!
    @brief  Set the software (bitbang) SPI MOSI line HIGH.
*/
INLINE void Arduino_SWSPI::SPI_MOSI_HIGH(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_mosiPortSet = 1;
#else // !KINETISK
  *_mosiPortSet = _mosiPinMask;
#endif
#else  // !HAS_PORT_SET_CLR
  *_mosiPort |= _mosiPinMaskSet;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_mosi, HIGH);
#endif // end !USE_FAST_PINIO
}

/*!
    @brief  Set the software (bitbang) SPI MOSI line LOW.
*/
INLINE void Arduino_SWSPI::SPI_MOSI_LOW(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_mosiPortClr = 1;
#else // !KINETISK
  *_mosiPortClr = _mosiPinMask;
#endif
#else  // !HAS_PORT_SET_CLR
  *_mosiPort &= _mosiPinMaskClr;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_mosi, LOW);
#endif // end !USE_FAST_PINIO
}

/*!
    @brief  Set the software (bitbang) SPI SCK line HIGH.
*/
INLINE void Arduino_SWSPI::SPI_SCK_HIGH(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_sckPortSet = 1;
#else                                                // !KINETISK
  *_sckPortSet = _sckPinMask;
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  for (volatile uint8_t i = 0; i < 1; i++)
    ;
#endif
#endif
#else  // !HAS_PORT_SET_CLR
  *_sckPort |= _sckPinMaskSet;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_sck, HIGH);
#endif // end !USE_FAST_PINIO
}

/*!
    @brief  Set the software (bitbang) SPI SCK line LOW.
*/
INLINE void Arduino_SWSPI::SPI_SCK_LOW(void)
{
#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
#if defined(KINETISK)
  *_sckPortClr = 1;
#else                                                // !KINETISK
  *_sckPortClr = _sckPinMask;
#if defined(__IMXRT1052__) || defined(__IMXRT1062__) // Teensy 4.x
  for (volatile uint8_t i = 0; i < 1; i++)
    ;
#endif
#endif
#else  // !HAS_PORT_SET_CLR
  *_sckPort &= _sckPinMaskClr;
#endif // end !HAS_PORT_SET_CLR
#else  // !USE_FAST_PINIO
  digitalWrite(_sck, LOW);
#endif // end !USE_FAST_PINIO
}

/*!
    @brief   Read the state of the software (bitbang) SPI MISO line.
    @return  true if HIGH, false if LOW.
*/
INLINE bool Arduino_SWSPI::SPI_MISO_READ(void)
{
#if defined(USE_FAST_PINIO)
#if defined(KINETISK)
  return *_misoPort;
#else  // !KINETISK
  return *_misoPort & _misoPinMask;
#endif // end !KINETISK
#else  // !USE_FAST_PINIO
  return digitalRead(_miso);
#endif // end !USE_FAST_PINIO
}