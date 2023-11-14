#ifndef _BLE_
#define _BLE_

#include "Arduino.h"
#include <WiFi.h>

#include "config.h"
#include "secrets.h"

#include <NimBLEDevice.h>

#define maxDistanceReadings 3

class BLE
{
  public:
    BLE();
    void begin();
    void loop();
    String getMessage();
    void setMessage( String heading );
    bool connectToServer();
    bool getServerValue();
    bool lookingTowardsEachOther( float avgDistance );
    void setRemoteHeading( float myh );
    float getRemoteHeading();
    void setLocalHeading( float myd );
    float getLocalHeading();
    bool matchHeading( float measured_angle ); 

  private:
    void areDevicesPointedToEachOther();

    std::string devname;
    int msgCounter;
    unsigned long serverWaitTime;
    unsigned long clientWaitTime;
    String message;
    NimBLECharacteristic* pHeadingCharacteristic;
    NimBLEServer* pServer;
    NimBLEClient* pClient;
    int runningDistance [ maxDistanceReadings ];
    float localHeading;
    float remoteHeading;
    String hottercolder;
    float oldAverageDistance;
};

#endif // _BLE_
