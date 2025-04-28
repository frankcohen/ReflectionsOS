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
#include "Wifi.h"

extern LOGGER logger;
extern Compass compass;
extern GPS gps;
extern RealTimeClock realtimeclock;
extern Wifi wifi;

static NimBLEServer* pServer;
static NimBLEService* pService;
static NimBLECharacteristic* pCharacteristic;
static NimBLEAdvertising* pAdvertising;    
static const NimBLEAdvertisedDevice* advDevice;
static bool doConnect  = false;
static uint32_t scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
static bool mypounce;

// Timing definitions (milliseconds)
#define scanInterval 1000    // Scan cycle in milliseconds
#define scanWindow 1000      // How long the scan waits
#define advInterval 10000    // Advertise every 10 seconds (10000 ms)

#define SCAN_TIME_MS            0       // 0 = scan forever
#define OWN_WATCHDOG_TIMEOUT    60000   // 20 seconds until our advertisement is considered stale
#define MAX_DEVICES             10      // Maximum number of remote devices to track
#define BLE_CLIENT_SCAN_TIME    60000   // Client scan duration
#define BLE_CLIENT_ATTEMPT_TIME 60000   // How long the client tries to connect

// Structure to hold remote device information.
struct ReflectionsData {
  String devicename;
  unsigned long lastUpdate;
  int heading;
  bool pounce;
  float latitude;
  float longitude;
  int rssi;
};

class BLEsupport 
{
  public:
    BLEsupport();

    void begin();         // Initialize BLE (server, advertising, scanning)
    void loop();          // Process BLE events (connections, watchdog, etc.)

    String getJsonData();
    void printRemoteDevices();  // For debugging: print list of remote devices
    unsigned long lastServerUpdate;    
    int getRemoteDevicesCount();  // Devices in BLE range
    bool isAnyDevicePounceTrue();

    void setPounce( bool pnc );
    bool getPounce();

    // Helper function used by the scan callback to connect to remote devices.
    void connectAndRead(NimBLEAdvertisedDevice* advertisedDevice);

    // Map to store remote device data; key is the device address.
    std::map<String, ReflectionsData> remoteDevices;
    
    // **Move ClientCallbacks here to public access**
    class ClientCallbacks : public NimBLEClientCallbacks 
    {
      public:
        void onConnect(NimBLEClient* pClient) override { Serial.printf(F("Connected\n")); }
        
        void onDisconnect(NimBLEClient* pClient, int reason) override 
        {
            Serial.printf(F("%s Disconnected, reason = %d - Starting scan\n"), pClient->getPeerAddress().toString().c_str(), reason);
            NimBLEDevice::getScan()->start(scanTimeMs, false, true);
        }
    };

  private:
    unsigned long compasstime;
    unsigned long lastAdvUpdate;
    unsigned long lastPrintTime;  // Timer for printing devices' data
    unsigned long pnctime;
    bool mypounce;

    bool connectToServer();

    void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

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
        void onScanEnd(const NimBLEScanResults& results, int reason) override;
      private:
        BLEsupport* _parent;
    } scanCallbacks;
    
};

#endif // BLE_SUPPORT_H
