/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef BLEsupport_H
#define BLEsupport_H

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
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

// Custom BLE service UUID and timing constants
#define REMOTE_WATCHDOG_TIMEOUT 30000   // 30 seconds: remote device stale if no update
#define OWN_WATCHDOG_TIMEOUT    20000   // 20 seconds: own update watchdog timeout
#define UPDATE_INTERVAL         15000   // Update and advertise every 15 seconds
#define MAX_DEVICES             10      // Maximum number of remote devices to track

// Structure to hold remote device information
struct DeviceInfo 
{
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
    void begin();  // Initialize BLE, advertising, and scanning
    void loop();   // Call this from the main loop to update advertisement and run watchdog checks

  private:
    int latestheading;
    bool gotAPounce;
    unsigned long pounceTimer;

    String devicename;

    // Updates the advertisement with the latest compass heading and pounce values.
    void updateAdvertisement();
    // Checks the watchdog timers (for own advertisement and remote devices).
    void checkWatchdog();

    // Remote device tracking
    DeviceInfo devices[MAX_DEVICES];
    int deviceCount;
    
    // Timing variable for our own advertisement update
    unsigned long lastOwnUpdate;
    // Pointers to BLEAdvertising and BLEScan instances
    BLEAdvertising* pAdvertising;
    BLEScan* pBLEScan;

    // Nested callback class for processing received advertisements.
    class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks 
    {
      public:
        MyAdvertisedDeviceCallbacks(BLEsupport* parent) : parentService(parent) {}
        void onResult(BLEAdvertisedDevice advertisedDevice) override;
      private:
        BLEsupport* parentService;
    };    

    // Helper method to update or add a remote device record.
    void updateDevice(const String& address, int heading, bool pounce, float latitude, float longitude);

};

#endif // BLEsupport_H
