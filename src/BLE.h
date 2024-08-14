#ifndef _BLE_
#define _BLE_

#include "Arduino.h"

#include "config.h"
#include "secrets.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

#include "Logger.h"
#include "TOF.h"
#include "Compass.h"

// Define BLEServerClass
class BLEServerClass 
{ 
  public:
    BLEServerClass();

    void begin();
    void handleClientData();
    void sendJsonData(String jsonData);

    int getLatestHeading();
    void setLatestHeading( int myh );
    bool isPounced();

    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pCharacteristic;
    BLEClient* pClient;
    BLERemoteCharacteristic* pRemoteCharacteristic;
    BLEAdvertising* pAdvertising;

    class MyServerCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    };

    class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic* pCharacteristic) override;
    };

  private:
    bool bleStarted;

};


// Define BLEClientClass
class BLEClientClass 
{
  public:
    BLEClientClass();
    void begin();
    void loop();
    void handleClientData();
    void sendJsonData(String jsonData);
    int getDistance();
    void sendPounce();
    
    BLEClient* pClient;
    BLERemoteCharacteristic* pRemoteCharacteristic;

    class MyClientCallbacks : public BLEClientCallbacks {
        void onConnect(BLEClient* pClient) override;
        void onDisconnect(BLEClient* pClient) override;
    };

  private:
    void clientConnect();
    int latestrssi;
    unsigned long sendHeadingTimer;
    unsigned long clientReconnectTimer;
    unsigned long checkRSSItimer;

};

#endif // _BLE_
