/*
 * start rewrite from:
 * https://github.com/daumemo/IPS_LCD_R61529_FT6236_Arduino_eSPI_Test
 */
#include "Arduino_ESP32PAR8QQ.h"

#if (CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2)

Arduino_ESP32PAR8QQ::Arduino_ESP32PAR8QQ(
    int8_t dc, int8_t cs, int8_t wr, int8_t rd,
    int8_t d0, int8_t d1, int8_t d2, int8_t d3, int8_t d4, int8_t d5, int8_t d6, int8_t d7)
    : _dc(dc), _cs(cs), _wr(wr), _rd(rd),
      _d0(d0), _d1(d1), _d2(d2), _d3(d3), _d4(d4), _d5(d5), _d6(d6), _d7(d7)
{
}

void Arduino_ESP32PAR8QQ::begin(int32_t speed, int8_t dataMode)
{
  pinMode(_dc, OUTPUT);
  digitalWrite(_dc, HIGH); // Data mode
  if (_dc >= 32)
  {
    _dcPinMask = digitalPinToBitMask(_dc);
    _dcPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _dcPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_dc >= 0)
  {
    _dcPinMask = digitalPinToBitMask(_dc);
    _dcPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _dcPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }

  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // disable chip select
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
    _csPinMask = 0;
    _csPortSet = _dcPortSet;
    _csPortClr = _dcPortClr;
  }

  pinMode(_wr, OUTPUT);
  digitalWrite(_wr, HIGH); // Set write strobe high (inactive)
  if (_wr >= 32)
  {
    _wrPinMask = digitalPinToBitMask(_wr);
    _wrPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _wrPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_wr >= 0)
  {
    _wrPinMask = digitalPinToBitMask(_wr);
    _wrPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _wrPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }

  if (_rd >= 0)
  {
    pinMode(_rd, OUTPUT);
    digitalWrite(_rd, HIGH);
  }
  if (_rd >= 32)
  {
    _rdPinMask = digitalPinToBitMask(_rd);
    _rdPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    _rdPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_rd >= 0)
  {
    _rdPinMask = digitalPinToBitMask(_rd);
    _rdPortSet = (PORTreg_t)&GPIO.out_w1ts;
    _rdPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  else
  {
    _rdPinMask = 0;
    _rdPortSet = _dcPortSet;
    _rdPortClr = _dcPortClr;
  }

  // TODO: check pin range 0-31
  pinMode(_d0, OUTPUT);
  pinMode(_d1, OUTPUT);
  pinMode(_d2, OUTPUT);
  pinMode(_d3, OUTPUT);
  pinMode(_d4, OUTPUT);
  pinMode(_d5, OUTPUT);
  pinMode(_d6, OUTPUT);
  pinMode(_d7, OUTPUT);

  // INIT 8-bit mask
  _dataClrMask = (1 << _wr) | (1 << _d0) | (1 << _d1) | (1 << _d2) | (1 << _d3) | (1 << _d4) | (1 << _d5) | (1 << _d6) | (1 << _d7);
  for (int32_t c = 0; c < 256; c++)
  {
    _xset_mask[c] = (1 << _wr);
    if (c & 0x01)
    {
      _xset_mask[c] |= (1 << _d0);
    }
    if (c & 0x02)
    {
      _xset_mask[c] |= (1 << _d1);
    }
    if (c & 0x04)
    {
      _xset_mask[c] |= (1 << _d2);
    }
    if (c & 0x08)
    {
      _xset_mask[c] |= (1 << _d3);
    }
    if (c & 0x10)
    {
      _xset_mask[c] |= (1 << _d4);
    }
    if (c & 0x20)
    {
      _xset_mask[c] |= (1 << _d5);
    }
    if (c & 0x40)
    {
      _xset_mask[c] |= (1 << _d6);
    }
    if (c & 0x80)
    {
      _xset_mask[c] |= (1 << _d7);
    }
  }
  _dataPortSet = (PORTreg_t)&GPIO.out_w1ts;
  _dataPortClr = (PORTreg_t)&GPIO.out_w1tc;
  *_dataPortClr = _dataClrMask;
}

void Arduino_ESP32PAR8QQ::beginWrite()
{
  DC_HIGH();
  CS_LOW();
}

void Arduino_ESP32PAR8QQ::endWrite()
{
  CS_HIGH();
}

void Arduino_ESP32PAR8QQ::writeCommand(uint8_t c)
{
  DC_LOW();

  WRITE(c);

  DC_HIGH();
}

void Arduino_ESP32PAR8QQ::writeCommand16(uint16_t c)
{
  DC_LOW();

  _data16.value = c;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);

  DC_HIGH();
}

void Arduino_ESP32PAR8QQ::write(uint8_t d)
{
  WRITE(d);
}

void Arduino_ESP32PAR8QQ::write16(uint16_t d)
{
  _data16.value = d;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);
}

void Arduino_ESP32PAR8QQ::writeRepeat(uint16_t p, uint32_t len)
{
  _data16.value = p;
  if (_data16.msb == _data16.lsb)
  {
    uint32_t setMask = _xset_mask[_data16.msb];
    *_dataPortClr = _dataClrMask;
    *_dataPortSet = setMask;
    while (len--)
    {
      *_wrPortClr = _wrPinMask;
      *_wrPortSet = _wrPinMask;
      *_wrPortClr = _wrPinMask;
      *_wrPortSet = _wrPinMask;
    }
  }
  else
  {
    uint32_t hiMask = _xset_mask[_data16.msb];
    uint32_t loMask = _xset_mask[_data16.lsb];
    while (len--)
    {
      *_dataPortClr = _dataClrMask;
      *_dataPortSet = hiMask;
      *_dataPortClr = _dataClrMask;
      *_dataPortSet = loMask;
    }
  }
}

void Arduino_ESP32PAR8QQ::writePixels(uint16_t *data, uint32_t len)
{
  while (len--)
  {
    _data16.value = *data++;
    WRITE(_data16.msb);
    WRITE(_data16.lsb);
  }
}

void Arduino_ESP32PAR8QQ::writeC8D8(uint8_t c, uint8_t d)
{
  DC_LOW();

  WRITE(c);

  DC_HIGH();

  WRITE(d);
}

void Arduino_ESP32PAR8QQ::writeC8D16(uint8_t c, uint16_t d)
{
  DC_LOW();

  WRITE(c);

  DC_HIGH();

  _data16.value = d;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);
}

void Arduino_ESP32PAR8QQ::writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2)
{
  DC_LOW();

  WRITE(c);

  DC_HIGH();

  _data16.value = d1;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);

  _data16.value = d2;
  WRITE(_data16.msb);
  WRITE(_data16.lsb);
}

void Arduino_ESP32PAR8QQ::writeBytes(uint8_t *data, uint32_t len)
{
  while (len--)
  {
    WRITE(*data++);
  }
}

void Arduino_ESP32PAR8QQ::writePattern(uint8_t *data, uint8_t len, uint32_t repeat)
{
  while (repeat--)
  {
    writeBytes(data, len);
  }
}

void Arduino_ESP32PAR8QQ::writeIndexedPixels(uint8_t *data, uint16_t *idx, uint32_t len)
{
  while (len--)
  {
    _data16.value = idx[*data++];
    WRITE(_data16.msb);
    WRITE(_data16.lsb);
  }
}

void Arduino_ESP32PAR8QQ::writeIndexedPixelsDouble(uint8_t *data, uint16_t *idx, uint32_t len)
{
  while (len--)
  {
    _data16.value = idx[*data++];
    WRITE(_data16.msb);
    WRITE(_data16.lsb);
    WRITE(_data16.msb);
    WRITE(_data16.lsb);
  }
}

INLINE void Arduino_ESP32PAR8QQ::WRITE(uint8_t d)
{
  uint32_t mask = _xset_mask[d];
  *_dataPortClr = _dataClrMask;
  *_dataPortSet = mask;
}

/******** low level bit twiddling **********/

INLINE void Arduino_ESP32PAR8QQ::DC_HIGH(void)
{
  *_dcPortSet = _dcPinMask;
}

INLINE void Arduino_ESP32PAR8QQ::DC_LOW(void)
{
  *_dcPortClr = _dcPinMask;
}

INLINE void Arduino_ESP32PAR8QQ::CS_HIGH(void)
{
  *_csPortSet = _csPinMask;
}

INLINE void Arduino_ESP32PAR8QQ::CS_LOW(void)
{
  *_csPortClr = _csPinMask;
}

#endif // #if (CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2)
