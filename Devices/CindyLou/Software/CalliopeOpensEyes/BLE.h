#ifndef _BLE_
#define _BLE_

#include "Arduino.h"
#include <WiFi.h>

#include "config.h"
#include "secrets.h"

#include <NimBLEDevice.h>

class BLE
{
  public:
    BLE();
    void begin();
    void loop();
    String getHeading();
    void setHeading( String heading );

  private:
    unsigned long serverWaitTime;
    unsigned long clientWaitTime;
    String heading;
};

#endif // _BLE_
