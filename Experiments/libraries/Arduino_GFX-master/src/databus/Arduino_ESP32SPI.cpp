/*
 * start rewrite from:
 * https://github.com/espressif/arduino-esp32.git
 */
#ifdef ESP32
#include "Arduino_DataBus.h"
#include "Arduino_ESP32SPI.h"

struct spi_struct_t
{
  spi_dev_t *dev;
#if !CONFIG_DISABLE_HAL_LOCKS
  xSemaphoreHandle lock;
#endif
  uint8_t num;
};

#if CONFIG_DISABLE_HAL_LOCKS
#define SPI_MUTEX_LOCK()
#define SPI_MUTEX_UNLOCK()

static spi_t _spi_bus_array[4] = {
    {(volatile spi_dev_t *)(DR_REG_SPI0_BASE), 0},
    {(volatile spi_dev_t *)(DR_REG_SPI1_BASE), 1},
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), 2},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), 3}};
#else // !CONFIG_DISABLE_HAL_LOCKS
#define SPI_MUTEX_LOCK() \
  do                     \
  {                      \
  } while (xSemaphoreTake(_spi->lock, portMAX_DELAY) != pdPASS)
#define SPI_MUTEX_UNLOCK() xSemaphoreGive(_spi->lock)

static spi_t _spi_bus_array[4] = {
    {(volatile spi_dev_t *)(DR_REG_SPI0_BASE), NULL, 0},
    {(volatile spi_dev_t *)(DR_REG_SPI1_BASE), NULL, 1},
    {(volatile spi_dev_t *)(DR_REG_SPI2_BASE), NULL, 2},
    {(volatile spi_dev_t *)(DR_REG_SPI3_BASE), NULL, 3}};
#endif // CONFIG_DISABLE_HAL_LOCKS

Arduino_ESP32SPI::Arduino_ESP32SPI(int8_t dc /* = -1 */, int8_t cs /* = -1 */, int8_t sck /* = -1 */, int8_t mosi /* = -1 */, int8_t miso /* = -1 */, uint8_t spi_num /* = VSPI */, bool is_shared_interface /* = true */)
    : _dc(dc), _spi_num(spi_num), _is_shared_interface(is_shared_interface)
{
  if (sck == -1 && miso == -1 && mosi == -1 && cs == -1)
  {
    _sck = (_spi_num == VSPI) ? SCK : 14;
    _miso = (_spi_num == VSPI) ? MISO : 12;
    _mosi = (_spi_num == VSPI) ? MOSI : 13;
    _cs = (_spi_num == VSPI) ? SS : 15;
  }
  else
  {
    _sck = sck;
    _miso = miso;
    _mosi = mosi;
    _cs = cs;
  }
}

static void _on_apb_change(void *arg, apb_change_ev_t ev_type, uint32_t old_apb, uint32_t new_apb)
{
  spi_t *_spi = (spi_t *)arg;
  if (ev_type == APB_BEFORE_CHANGE)
  {
    SPI_MUTEX_LOCK();
    while (_spi->dev->cmd.usr)
      ;
  }
  else
  {
    _spi->dev->clock.val = spiFrequencyToClockDiv(old_apb / ((_spi->dev->clock.clkdiv_pre + 1) * (_spi->dev->clock.clkcnt_n + 1)));
    SPI_MUTEX_UNLOCK();
  }
}

void Arduino_ESP32SPI::begin(int32_t speed, int8_t dataMode)
{
  _speed = speed ? speed : SPI_DEFAULT_FREQ;
  _dataMode = dataMode;

  if (!_div)
  {
    _div = spiFrequencyToClockDiv(_speed);
  }

  if (_dc >= 0)
  {
    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH); // Data mode
  }
  if (_dc >= 32)
  {
    dcPinMask = digitalPinToBitMask(_dc);
    dcPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    dcPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_dc >= 0)
  {
    dcPinMask = digitalPinToBitMask(_dc);
    dcPortSet = (PORTreg_t)&GPIO.out_w1ts;
    dcPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }

  if (_cs >= 0)
  {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // disable chip select
  }
  if (_cs >= 32)
  {
    csPinMask = digitalPinToBitMask(_cs);
    csPortSet = (PORTreg_t)&GPIO.out1_w1ts.val;
    csPortClr = (PORTreg_t)&GPIO.out1_w1tc.val;
  }
  else if (_cs >= 0)
  {
    csPinMask = digitalPinToBitMask(_cs);
    csPortSet = (PORTreg_t)&GPIO.out_w1ts;
    csPortClr = (PORTreg_t)&GPIO.out_w1tc;
  }
  else
  {
    csPinMask = 0;
    csPortSet = dcPortSet;
    csPortClr = dcPortClr;
  }

  // SPI.begin(_sck, _miso, _mosi);
  // _spi = spiStartBus(_spi_num, _div, SPI_MODE0, SPI_MSBFIRST);
  _spi = &_spi_bus_array[_spi_num];

#if !CONFIG_DISABLE_HAL_LOCKS
  if (_spi->lock == NULL)
  {
    _spi->lock = xSemaphoreCreateMutex();
  }
#endif

  if (_spi_num == HSPI)
  {
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST);
  }
  else if (_spi_num == VSPI)
  {
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN_2);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST_2);
  }
  else
  {
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN_1);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST_1);
  }

  spiStopBus(_spi);
  spiSetDataMode(_spi, _dataMode);
  spiSetBitOrder(_spi, _bitOrder);
  spiSetClockDiv(_spi, _div);

  SPI_MUTEX_LOCK();
  _spi->dev->user.usr_mosi = 1;
  if (_miso < 0)
  {
    _spi->dev->user.usr_miso = 0;
    _spi->dev->user.doutdin = 0;
  }
  else
  {
    _spi->dev->user.usr_miso = 1;
    _spi->dev->user.doutdin = 1;
  }

  for (uint8_t i = 0; i < 16; i++)
  {
    _spi->dev->data_buf[i] = 0x00000000;
  }
  SPI_MUTEX_UNLOCK();

  addApbChangeCallback(_spi, _on_apb_change);

  spiAttachSCK(_spi, _sck);

  if (_miso >= 0)
  {
    spiAttachMISO(_spi, _miso);
  }

  spiAttachMOSI(_spi, _mosi);

  if (!_is_shared_interface)
  {
    spiTransaction(_spi, _div, _dataMode, _bitOrder);
  }
}

void Arduino_ESP32SPI::beginWrite()
{
  data_buf_bit_idx = 0;
  data_buf[0] = 0;

  if (_is_shared_interface)
  {
    spiTransaction(_spi, _div, _dataMode, _bitOrder);
  }

  if (_dc >= 0)
  {
    DC_HIGH();
  }
  CS_LOW();
}

void Arduino_ESP32SPI::writeCommand(uint8_t c)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(c);
  }
  else
  {
    if (data_buf_bit_idx > 0)
    {
      flush_data_buf();
    }

    DC_LOW();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = c;
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;

    DC_HIGH();
  }
}

void Arduino_ESP32SPI::writeCommand16(uint16_t c)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(c >> 8);
    WRITE9BIT(c & 0xff);
  }
  else
  {
    if (data_buf_bit_idx > 0)
    {
      flush_data_buf();
    }

    DC_LOW();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 15;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    MSB_16_SET(_spi->dev->data_buf[0], c);
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;

    DC_HIGH();
  }
}

void Arduino_ESP32SPI::writeCommand32(uint32_t c)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(c >> 24);
    WRITE9BIT((c >> 16) & 0xff);
    WRITE9BIT((c >> 8) & 0xff);
    WRITE9BIT(c & 0xff);
  }
  else
  {
    if (data_buf_bit_idx > 0)
    {
      flush_data_buf();
    }

    DC_LOW();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 31;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    MSB_32_SET(_spi->dev->data_buf[0], c);
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;

    DC_HIGH();
  }
}

void Arduino_ESP32SPI::write(uint8_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(0x100 | d);
  }
  else
  {
    WRITE8BIT(d);
  }
}

void Arduino_ESP32SPI::write16(uint16_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(0x100 | (d >> 8));
    WRITE9BIT(0x100 | (d & 0xff));
  }
  else
  {
    WRITE8BIT(d >> 8);
    WRITE8BIT(d);
  }
}

void Arduino_ESP32SPI::write32(uint32_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(0x100 | (d >> 24));
    WRITE9BIT(0x100 | ((d >> 16) & 0xff));
    WRITE9BIT(0x100 | ((d >> 8) & 0xff));
    WRITE9BIT(0x100 | (d & 0xff));
  }
  else
  {
    WRITE8BIT(d >> 24);
    WRITE8BIT(d >> 16);
    WRITE8BIT(d >> 8);
    WRITE8BIT(d);
  }
}

void Arduino_ESP32SPI::writeC8D8(uint8_t c, uint8_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(c);
    WRITE9BIT(0x100 | d);
  }
  else
  {
    if (data_buf_bit_idx > 0)
    {
      flush_data_buf();
    }

    DC_LOW();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = c;
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;

    DC_HIGH();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = d;
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;
  }
}

void Arduino_ESP32SPI::writeC8D16(uint8_t c, uint16_t d)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(c);
    WRITE9BIT(0x100 | (d >> 8));
    WRITE9BIT(0x100 | (d & 0xff));
  }
  else
  {
    if (data_buf_bit_idx > 0)
    {
      flush_data_buf();
    }

    DC_LOW();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = c;
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;

    DC_HIGH();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 15;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = (d >> 8) | ((d & 0xff) << 8);
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;
  }
}

void Arduino_ESP32SPI::writeC8D16D16(uint8_t c, uint16_t d1, uint16_t d2)
{
  if (_dc < 0) // 9-bit SPI
  {
    WRITE9BIT(c);
    WRITE9BIT(0x100 | (d1 >> 8));
    WRITE9BIT(0x100 | (d1 & 0xff));
    WRITE9BIT(0x100 | (d2 >> 8));
    WRITE9BIT(0x100 | (d2 & 0xff));
  }
  else
  {
    if (data_buf_bit_idx > 0)
    {
      flush_data_buf();
    }

    DC_LOW();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 7;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = c;
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;

    DC_HIGH();

    _spi->dev->mosi_dlen.usr_mosi_dbitlen = 31;
    _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
    _spi->dev->data_buf[0] = (d1 >> 8) | ((d1 & 0xff) << 8) | ((d2 & 0xff00) << 8) | ((d2 & 0xff) << 24);
    _spi->dev->cmd.usr = 1;
    while (_spi->dev->cmd.usr)
      ;
  }
}

void Arduino_ESP32SPI::endWrite()
{
  if (data_buf_bit_idx > 0)
  {
    flush_data_buf();
  }

  if (_is_shared_interface)
  {
    spiEndTransaction(_spi);
  }

  CS_HIGH();
}

void Arduino_ESP32SPI::sendCommand(uint8_t c)
{
  beginWrite();

  writeCommand(c);

  endWrite();
}

void Arduino_ESP32SPI::sendCommand16(uint16_t c)
{
  beginWrite();

  writeCommand16(c);

  endWrite();
}

void Arduino_ESP32SPI::sendCommand32(uint32_t c)
{
  beginWrite();

  writeCommand32(c);

  endWrite();
}

void Arduino_ESP32SPI::sendData(uint8_t d)
{
  beginWrite();

  write(d);

  endWrite();
}

void Arduino_ESP32SPI::sendData16(uint16_t d)
{
  beginWrite();

  write16(d);

  endWrite();
}

void Arduino_ESP32SPI::sendData32(uint32_t d)
{
  beginWrite();

  write32(d);

  endWrite();
}

void Arduino_ESP32SPI::writeRepeat(uint16_t p, uint32_t len)
{
  if (data_buf_bit_idx > 0)
  {
    flush_data_buf();
  }

  if (_dc < 0) // 9-bit SPI
  {
    uint32_t hi = 0x100 | (p >> 8);
    uint32_t lo = 0x100 | (p & 0xff);
    uint16_t idx;
    uint8_t shift;
    uint32_t l;
    uint16_t bufLen = (len <= 28) ? len : 28;
    uint16_t xferLen;
    for (uint32_t t = 0; t < bufLen; t++)
    {
      idx = data_buf_bit_idx >> 3;
      shift = (data_buf_bit_idx % 8);
      if (shift)
      {
        data_buf[idx++] |= hi >> (shift + 1);
        data_buf[idx] = hi << (7 - shift);
      }
      else
      {
        data_buf[idx++] = hi >> 1;
        data_buf[idx] = hi << 7;
      }
      data_buf_bit_idx += 9;

      idx = data_buf_bit_idx >> 3;
      shift = (data_buf_bit_idx % 8);
      if (shift)
      {
        data_buf[idx++] |= lo >> (shift + 1);
        data_buf[idx] = lo << (7 - shift);
      }
      else
      {
        data_buf[idx++] = lo >> 1;
        data_buf[idx] = lo << 7;
      }
      data_buf_bit_idx += 9;
    }

    if (_miso < 0)
    {
      l = (data_buf_bit_idx + 31) / 32;
      for (uint32_t i = 0; i < l; i++)
      {
        _spi->dev->data_buf[i] = data_buf32[i];
      }
    }

    // Issue pixels in blocks from temp buffer
    while (len)
    {                                          // While pixels remain
      xferLen = (bufLen < len) ? bufLen : len; // How many this pass?
      data_buf_bit_idx = xferLen * 18;
      _spi->dev->mosi_dlen.usr_mosi_dbitlen = data_buf_bit_idx - 1;
      _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
      if (_miso >= 0)
      {
        l = (data_buf_bit_idx + 31) / 32;
        for (uint32_t i = 0; i < l; i++)
        {
          _spi->dev->data_buf[i] = data_buf32[i];
        }
      }
      _spi->dev->cmd.usr = 1;
      while (_spi->dev->cmd.usr)
        ;
      len -= xferLen;
    }
  }
  else // 8-bit SPI
  {
    uint16_t bufLen = (len < 32) ? len : 32;
    uint16_t xferLen, l;
    uint32_t c32;
    MSB_32_16_16_SET(c32, p, p);

    if (_miso < 0)
    {
      l = (bufLen + 1) / 2;
      for (uint32_t i = 0; i < l; i++)
      {
        _spi->dev->data_buf[i] = c32;
      }
    }

    // Issue pixels in blocks from temp buffer
    while (len)
    {                                           // While pixels remain
      xferLen = (bufLen <= len) ? bufLen : len; // How many this pass?
      _spi->dev->mosi_dlen.usr_mosi_dbitlen = (xferLen * 16) - 1;
      _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
      if (_miso >= 0)
      {
        l = (xferLen + 1) / 2;
        for (uint32_t i = 0; i < l; i++)
        {
          _spi->dev->data_buf[i] = c32;
        }
      }
      _spi->dev->cmd.usr = 1;
      while (_spi->dev->cmd.usr)
        ;
      len -= xferLen;
    }
  }

  data_buf_bit_idx = 0;
}

void Arduino_ESP32SPI::writeBytes(uint8_t *data, uint32_t len)
{
  if (_dc < 0) // 9-bit SPI
  {
    while (len--)
    {
      write(*data++);
    }
  }
  else // 8-bit SPI
  {
    uint32_t *p = (uint32_t *)data;
    if (len >= 64)
    {
      if (data_buf_bit_idx > 0)
      {
        flush_data_buf();
      }
      _spi->dev->mosi_dlen.usr_mosi_dbitlen = 511;
      _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
      while (len >= 64)
      {
        for (uint32_t i = 0; i < 16; i++)
        {
          _spi->dev->data_buf[i] = *p++;
        }
        _spi->dev->cmd.usr = 1;
        while (_spi->dev->cmd.usr)
          ;

        len -= 64;
      }
    }

    if ((len > 0) && ((len % 4) == 0))
    {
      if (data_buf_bit_idx > 0)
      {
        flush_data_buf();
      }

      _spi->dev->mosi_dlen.usr_mosi_dbitlen = (len * 8) - 1;
      _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
      len >>= 2; // 4 bytes to a 32-bit data
      for (uint32_t i = 0; i < len; i++)
      {
        _spi->dev->data_buf[i] = *p++;
      }
      _spi->dev->cmd.usr = 1;
      while (_spi->dev->cmd.usr)
        ;
    }
    else
    {
      while (len--)
      {
        write(*data++);
      }
    }
  }
}

void Arduino_ESP32SPI::writePixels(uint16_t *data, uint32_t len)
{
  if (_dc < 0) // 9-bit SPI
  {
    while (len--)
    {
      write16(*data++);
    }
  }
  else // 8-bit SPI
  {
    if (len >= 32)
    {
      if (data_buf_bit_idx > 0)
      {
        flush_data_buf();
      }

      _spi->dev->mosi_dlen.usr_mosi_dbitlen = 511;
      _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
      uint32_t v1, v2;
      while (len >= 32)
      {
        for (uint8_t i = 0; i < 16; i++)
        {
          v1 = *data++;
          v2 = *data++;
          _spi->dev->data_buf[i] = ((v1 & 0xff00) >> 8) | ((v1 & 0xff) << 8) | ((v2 & 0xff00) << 8) | ((v2 & 0xff) << 24);
        }
        _spi->dev->cmd.usr = 1;
        while (_spi->dev->cmd.usr)
          ;

        len -= 32;
      }
    }

    if ((len > 0) && ((len % 2) == 0))
    {
      if (data_buf_bit_idx > 0)
      {
        flush_data_buf();
      }

      _spi->dev->mosi_dlen.usr_mosi_dbitlen = (len * 16) - 1;
      _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
      uint32_t v1, v2;
      len >>= 1; // 2 pixels to a 32-bit data
      for (uint32_t i = 0; i < len; i++)
      {
        v1 = *data++;
        v2 = *data++;
        _spi->dev->data_buf[i] = ((v1 & 0xff00) >> 8) | ((v1 & 0xff) << 8) | ((v2 & 0xff00) << 8) | ((v2 & 0xff) << 24);
      }
      _spi->dev->cmd.usr = 1;
      while (_spi->dev->cmd.usr)
        ;
    }
    else
    {
      while (len--)
      {
        write16(*data++);
      }
    }
  }
}

/**
 * @param data uint8_t *
 * @param len uint8_t
 * @param repeat uint32_t
 */
void Arduino_ESP32SPI::writePattern(uint8_t *data, uint8_t len, uint32_t repeat)
{
  while (repeat--)
  {
    writeBytes(data, len);
  }
}

void Arduino_ESP32SPI::flush_data_buf()
{
  _spi->dev->mosi_dlen.usr_mosi_dbitlen = data_buf_bit_idx - 1;
  _spi->dev->miso_dlen.usr_miso_dbitlen = 0;
  uint32_t len = (data_buf_bit_idx + 31) / 32;
  for (uint32_t i = 0; i < len; i++)
  {
    _spi->dev->data_buf[i] = data_buf32[i];
  }
  _spi->dev->cmd.usr = 1;
  while (_spi->dev->cmd.usr)
    ;
  data_buf_bit_idx = 0;
}

INLINE void Arduino_ESP32SPI::WRITE8BIT(uint8_t d)
{
  uint16_t idx = data_buf_bit_idx >> 3;
  data_buf[idx] = d;
  data_buf_bit_idx += 8;
  if (data_buf_bit_idx >= 512)
  {
    flush_data_buf();
  }
}

INLINE void Arduino_ESP32SPI::WRITE9BIT(uint32_t d)
{
  uint16_t idx = data_buf_bit_idx >> 3;
  uint8_t shift = (data_buf_bit_idx % 8);
  if (shift)
  {
    data_buf[idx++] |= d >> (shift + 1);
    data_buf[idx] = d << (7 - shift);
  }
  else
  {
    data_buf[idx++] = d >> 1;
    data_buf[idx] = d << 7;
  }
  data_buf_bit_idx += 9;
  if (data_buf_bit_idx >= 504) // 56 bytes * 9 bits
  {
    flush_data_buf();
  }
}

/******** low level bit twiddling **********/

INLINE void Arduino_ESP32SPI::DC_HIGH(void)
{
  *dcPortSet = dcPinMask;
}

INLINE void Arduino_ESP32SPI::DC_LOW(void)
{
  *dcPortClr = dcPinMask;
}

INLINE void Arduino_ESP32SPI::CS_HIGH(void)
{
  *csPortSet = csPinMask;
}

INLINE void Arduino_ESP32SPI::CS_LOW(void)
{
  *csPortClr = csPinMask;
}

#endif
