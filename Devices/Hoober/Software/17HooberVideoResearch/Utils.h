#ifndef _UTILS_
#define _UTILS_

#include "SD.h"
#include "FS.h"
#include "config.h"

#include "WiFi.h"
#include "WiFiMulti.h"

class Utils
{
  public:
    Utils();
    void begin();
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    void startSDWifi();
    void loop();

  private:
    WiFiMulti _wifiMulti;
    long _connectionTimer;

};

#endif // _UTILS_
