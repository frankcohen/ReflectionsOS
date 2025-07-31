/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 ChatGPT says RSSI values:
 -30 dBm to -50 dBm: Very strong signal; the devices are very close (within a few centimeters to about 1 meter).
 -50 dBm to -70 dBm: Strong signal; the devices are likely within 1 to 5 meters.
 -70 dBm to -90 dBm: Moderate signal; the devices might be within 5 to 10 meters.
 -90 dBm and below: Weak signal; the devices are likely farther apart or there are significant obstacles/interference.

*/

#ifndef BLE_SUPPORT_H
#define BLE_SUPPORT_H

#include "NimBLEDevice.h"
#include "NimBLEScan.h"             // Contains NimBLEScanCallbacks in 2.2.3
#include "NimBLEAdvertisementData.h"
#include "NimBLEAdvertising.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include <map>
#include <set>
#include <vector>

#include "config.h"
#include "Logger.h"
#include "Compass.h"
#include "GPS.h"
#include "RealTimeClock.h"

extern LOGGER logger;
extern Compass compass;
extern GPS gps;
extern RealTimeClock realtimeclock;
extern Wifi wifi;

#define POUNCETIMEOUT           10000   // Timeout pounce

struct PounceData {
    bool pounce;
    unsigned long timestamp;
};

struct DeviceConnection {
    unsigned long timestamp;
};

class BLEsupport {
  public:
    BLEsupport();
    void begin();
    void loop();
    
    // Function to send JSON data to connected clients
    void sendJsonData(const JsonDocument& doc);
    // Function to read JSON data from the characteristic
    bool readJsonData(JsonDocument& doc);

    // Function to print the list of remote devices matching the UUIDs
    void printRemoteDevices();
    
    // Function to set heading, pounce, latitude, and longitude in JSON data
    void setJsonData(const String& devicename, float heading, bool pounce, float latitude, float longitude);

    // Function to check if any device has sent a pounce value of true in the last 10 seconds
    bool isAnyDevicePounceTrue();

    void setPounce( bool pnc );
    bool getPounce();

    int getRemoteDevicesCount();

  private:
    NimBLEServer* pServer;
    NimBLEService* pService;
    NimBLECharacteristic* pCharacteristic;
    NimBLEAdvertising* pAdvertising;

    std::vector<PounceData> pounceDataList;

    // MAC addresses of unique devices that have connected
    std::vector<std::string> uniqueDeviceAddresses;

    // Timestamp of the last received pounce message with pounce = true
    unsigned long lastPounceTimestamp;

    // Function to handle BLE client connections and data transfer
    void handleBLEConnections();
    // Function to setup the server and characteristic
    void setupServer();
    
    // Function to handle device scan results
    void scanForDevices();

    // JSON document for sending data
    StaticJsonDocument<200> jsonData;

    bool mypounce;
    unsigned long pnctime;
};


#endif // BLE_SUPPORT_H
