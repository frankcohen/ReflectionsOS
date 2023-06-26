#ifndef _UTILS_
#define _UTILS_

#include <SPI.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "config.h"
#include "secrets.h"

#include "Wire.h"

#include "WiFi.h"
#include "WiFiMulti.h"

class Utils
{
  public:
    Utils();
    void begin();
    void loop();
    void WireScan();

  private:
    WiFiMulti _wifiMulti;
    long _connectionTimer;
};

#endif // _UTILS_
