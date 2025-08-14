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

#include "config.h"
#include "Logger.h"
#include "Compass.h"
#include "GPS.h"
#include "RealTimeClock.h"

#define scanTimeMs 5000

extern LOGGER logger;
extern Compass compass;
extern GPS gps;
extern RealTimeClock realtimeclock;
extern Wifi wifi;

static String s_devicename;
static bool   s_pounce;
static float  s_heading;
static float  s_latitude;
static float  s_longitude;
static int    s_rssi;
static unsigned long s_when;

static bool pounced;

struct ClientData {
    NimBLEClient* pClient;
    unsigned long lastActivityTime;  // Store the last activity time for the client
};

class BLEsupport {
  public:
    BLEsupport();
    void begin();
    void loop();

    bool isCatNearby();
    bool isPounced();
    void sendPounce();
    int getRSSI();

    bool getPounce();
    float getHeading();
    float getLatitude();
    float getLongitude();
    String getDevicename();

  private:
    bool connectToServer();
    void setupServer();
    void handleServerConnections();
    int mynum;
    unsigned long mytime;    
};

#endif // BLE_SUPPORT_H
