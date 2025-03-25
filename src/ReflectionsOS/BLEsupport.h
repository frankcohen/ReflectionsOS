/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef BLE_SUPPORT_H
#define BLE_SUPPORT_H

#include <Arduino.h>
#include "NimBLEDevice.h"
#include "NimBLEScan.h"             // Contains NimBLEScanCallbacks in 2.2.3
#include "NimBLEAdvertisementData.h"
#include "NimBLEAdvertising.h"
#include <ArduinoJson.h>

#include <map>
#include <set>
#include <vector>

#include "config.h"
#include "secrets.h"
#include "Logger.h"
#include "Compass.h"
#include "GPS.h"
#include "RealTimeClock.h"

extern LOGGER logger;
extern Compass compass;
extern GPS gps;
extern RealTimeClock realtimeclock;
extern Wifi wifi;

// Timing definitions (milliseconds)
#define SCAN_TIME_MS            0       // 0 = scan forever
#define OWN_WATCHDOG_TIMEOUT    40000   // 20 seconds until our advertisement is considered stale
#define MAX_DEVICES             10      // Maximum number of remote devices to track
#define BLE_CLIENT_SCAN_TIME    20000   // Client scan duration
#define BLE_CLIENT_ATTEMPT_TIME 20000   // How long the client tries to connect

// Structure to hold remote device information.
struct ReflectionsData {
  String devicename;
  unsigned long lastUpdate;
  int heading;
  bool pounce;
  float latitude;
  float longitude;
};

class BLEsupport 
{
  public:
    BLEsupport();
    void begin();         // Initialize BLE (server, advertising, scanning)
    void loop();          // Process BLE events (connections, watchdog, etc.)
    void watchdogCheck(); // Check local and remote watchdog timers
    String getJsonData();
    void printRemoteDevices();  // For debugging: print list of remote devices

    // Helper function used by the scan callback to connect to remote devices.
    void connectAndRead(NimBLEAdvertisedDevice* advertisedDevice);

    // Map to store remote device data; key is the device address.
    std::map<String, ReflectionsData> remoteDevices;
    
  private:
    unsigned long compasstime;
    unsigned long lastAdvUpdate;

    // Server BLE objects.
    NimBLEServer* pServer;
    NimBLEService* pService;
    NimBLECharacteristic* pCharacteristic; // Local server characteristic.
    NimBLEAdvertising* pAdvertising;    

    // ----------------- SERVER CALLBACK -----------------
    class MyCharacteristicCallbacks : public NimBLECharacteristicCallbacks 
    {
      public:
        MyCharacteristicCallbacks(BLEsupport* parent);
        void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
      private:
        BLEsupport* _parent;
    };

    // ----------------- CLIENT SCAN CALLBACK -----------------
    // Use NimBLEScanCallbacks (available in 2.2.3).
    class ScanCallbacks : public NimBLEScanCallbacks {
      public:
        // Constructor takes a pointer to the parent BLEsupport instance.
        ScanCallbacks(BLEsupport* parent);
        void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
      private:
        BLEsupport* _parent;
    };

    // Member variable for scan callbacks (initialized in the constructor’s initializer list).
    ScanCallbacks scanCallbacks;
};

#endif // BLE_SUPPORT_H
