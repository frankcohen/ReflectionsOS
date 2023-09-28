#pragma once

#if defined(ARDUINO_ARCH_SAMD) 
#include "AudioI2S/I2SConfig.h"
#include <I2S.h>


namespace audio_tools {

/**
 * @brief Basic I2S API - for the SAMD
 * @ingroup platform
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class I2SDriverSAMD {
  friend class I2SStream;

  public:

    /// Provides the default configuration
    I2SConfig defaultConfig(RxTxMode mode) {
        I2SConfig c(mode);
        return c;
    }

    /// starts the DAC with the default config
    bool begin(RxTxMode mode) {
      return begin(defaultConfig(mode));
    }

    /// starts the DAC 
    bool begin(I2SConfig cfg) {
        this->cfg = cfg;
        return I2S.begin(cfg.i2s_format, cfg.sample_rate, cfg.bits_per_sample);
    }

    bool begin() {
      return begin(cfg);
    }

    /// stops the I2C and unistalls the driver
    void end(){
        I2S.end();
    }

    /// provides the actual configuration
    I2SConfig config() {
      return cfg;
    }

    size_t writeBytes(const void *src, size_t size_bytes){
      return I2S.write((const uint8_t *)src, size_bytes);
    }

    size_t readBytes(void *src, size_t size_bytes){
      return I2S.read(src, size_bytes);
    }

    int available() {
      return I2S.available();
    }

    int availableForWrite() {
      return I2S.availableForWrite();
    }

  protected:
    I2SConfig cfg;
    
};

using I2SDriver = I2SDriverSAMD;

}

#endif
