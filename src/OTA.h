#ifndef _OTA_
#define _OTA_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"

#include "esp_ota_ops.h"        //included header file for esp_ota driver
#include "FS.h"
#include "SD.h"

#include <WiFiUdp.h>
#include <WiFiMulti.h>

class OTA
{
  public:
    OTA();
    void begin();
    bool update();
    void loop();

  private:

};

#endif // _OTA_
