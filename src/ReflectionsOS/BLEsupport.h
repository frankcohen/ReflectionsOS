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

#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h> // This header defines NimBLEAdvertisedDevice and its callbacks.

#include <ArduinoJson.h>

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

// Custom BLE service UUID and timing constants
#define REMOTE_WATCHDOG_TIMEOUT 30000   // 30 seconds: remote device stale if no update
#define OWN_WATCHDOG_TIMEOUT    20000   // 20 seconds: own update watchdog timeout
#define UPDATE_INTERVAL         15000   // Update and advertise every 15 seconds
#define MAX_DEVICES             10      // Maximum number of remote devices to track

static const NimBLEAdvertisedDevice* advDevice;
static bool                          doConnect  = false;
static uint32_t                      scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */

// Structure to hold remote device information (including GPS location)
struct DeviceInfo {
  String address;
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
    void begin();  // Initialize NimBLE, advertising, and scanning
    void loop();   // Call from the main loop to update advertisement and run watchdog checks

  private:
    // Updates the advertisement with the latest compass heading, pounce, and GPS location.
    void updateAdvertisement();
    // Checks the watchdog timers (for our own advertisement and remote devices).
    void checkWatchdog();
    // Updates or adds a remote device record.
    void updateDevice(const String &address, int heading, bool pounce, float latitude, float longitude);

    String devicename;

    // Remote device tracking
    DeviceInfo devices[MAX_DEVICES];
    int deviceCount;
    
    // Timing variable for our own advertisement update
    unsigned long lastOwnUpdate;
    // Pointers to NimBLE advertising and scanning instances
    NimBLEAdvertising* pAdvertising;
    NimBLEScan* pBLEScan;

};

#endif // BLE_SUPPORT_H
