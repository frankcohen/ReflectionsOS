/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

*/

#include "BLEsupport.h"

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
   // Called for each advertising BLE server
   
  void onResult( NimBLEAdvertisedDevice* advertisedDevice ) 
  {
    BLEUUID serviceUUID( BLE_SERVICE_UUID );

    if (advertisedDevice->haveServiceData() &&
      advertisedDevice->isAdvertisingService(NimBLEUUID( BLE_SERVICE_UUID ))) 
    {
      // Retrieve the service data (convert from std::string to Arduino String).
      std::string serviceDataStd = advertisedDevice->getServiceData();
      String serviceData = String(serviceDataStd.c_str());

      DynamicJsonDocument doc(128);
      DeserializationError error = deserializeJson(doc, serviceData.c_str());
      if (error) 
      {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
      }

      int receivedHeading = doc["h"];
      bool receivedPounce = doc["p"];
      float receivedLat = doc["lat"];
      float receivedLon = doc["lon"];
      String devAddress = advertisedDevice->getAddress().toString().c_str();

      Serial.print("Received from ");
      Serial.print(devAddress);
      Serial.print(" - Heading: ");
      Serial.print(receivedHeading);
      Serial.print(", Pounce: ");
      Serial.print(receivedPounce ? "true" : "false");
      Serial.print(", Latitude: ");
      Serial.print(receivedLat);
      Serial.print(", Longitude: ");
      Serial.println(receivedLon);
      Serial.print( ", RSSI: " );
      Serial.println( advertisedDevice -> getRSSI() );
    }
  }
} advertisedDeviceCallbacks;

/** Define a class to handle the callbacks when scan events are received */
class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        Serial.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
        if (advertisedDevice->isAdvertisingService(NimBLEUUID("DEAD"))) {
            Serial.printf("Found Our Service\n");
            /** stop scan before connecting */
            NimBLEDevice::getScan()->stop();
            /** Save the device reference in a global for the client to use*/
            advDevice = advertisedDevice;
            /** Ready to connect now */
            doConnect = true;
        }
    }

    /** Callback to process the results of the completed scan or restart it */
    void onScanEnd(const NimBLEScanResults& results, int reason) override {
        Serial.printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} scanCallbacks;

BLEsupport::BLEsupport(){}

void BLEsupport::begin() 
{
  Serial.println("BLE starting");

  NimBLEDevice::init( wifi.getDeviceName() );

  pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData advData;
  advData.setName( wifi.getDeviceName() );
  
  int heading = compass.getHeading();
  bool pounce = false;
  float latitude = gps.getLat();
  float longitude = gps.getLng();
  
  DynamicJsonDocument doc(128);
  doc["h"] = heading;
  doc["p"] = pounce;
  doc["lat"] = latitude;
  doc["lon"] = longitude;
  String jsonData;
  serializeJson(doc, jsonData);
  
  advData.setServiceData( NimBLEUUID( BLE_SERVICE_UUID ), jsonData.c_str() );
  pAdvertising->setAdvertisementData(advData);
  
  NimBLEAdvertisementData scanData;
  scanData.setName( wifi.getDeviceName() );
  pAdvertising->setScanResponseData( scanData );
  
  pAdvertising->start();
  Serial.println( "BLE advertising started" );

  // --- Setup NimBLE Scanning ---
  pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setScanCallbacks( new MyAdvertisedDeviceCallbacks() );
  pBLEScan->setActiveScan(true);
  // Start scanning continuously (duration 0 means continuous scanning).
  pBLEScan->start(0, false, true);

  lastOwnUpdate = millis();

  Serial.println( "BLE started" );
}

void BLEsupport::updateAdvertisement() 
{
  int heading = compass.getHeading();
  bool pounce = false;
  float latitude = gps.getLat();
  float longitude = gps.getLng();

  Serial.print("Updating advertisement - Heading: ");
  Serial.print(heading);
  Serial.print(", Pounce: ");
  Serial.print(pounce ? "true" : "false");
  Serial.print(", Latitude: ");
  Serial.print(latitude);
  Serial.print(", Longitude: ");
  Serial.println(longitude);

  DynamicJsonDocument doc(128);
  doc["h"] = heading;
  doc["p"] = pounce;
  doc["lat"] = latitude;
  doc["lon"] = longitude;
  String jsonData;
  serializeJson(doc, jsonData);

  NimBLEAdvertisementData advData;
  advData.setName( wifi.getDeviceName() );
  advData.setServiceData( NimBLEUUID( BLE_SERVICE_UUID ), jsonData.c_str());

  pAdvertising->stop();  // Stop advertising to update data.
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->start();
}

void BLEsupport::checkWatchdog() {
  unsigned long currentMillis = millis();

  // --- Own Advertisement Watchdog ---
  if (currentMillis - lastOwnUpdate >= OWN_WATCHDOG_TIMEOUT) {
    Serial.println("Warning: Own advertisement not updated within expected interval!");
  }

  // --- Remote Devices Watchdog ---
  for (int i = 0; i < deviceCount; i++) {
    if (currentMillis - devices[i].lastUpdate >= REMOTE_WATCHDOG_TIMEOUT) {
      Serial.print("Warning: Lost updates from device ");
      Serial.print(devices[i].address);
      Serial.println(" - no update received recently!");
      // Remove the stale device entry by shifting remaining elements.
      for (int j = i; j < deviceCount - 1; j++) {
        devices[j] = devices[j + 1];
      }
      deviceCount--;
      i--;  // Adjust index after removal.
    }
  }
}

void BLEsupport::updateDevice(const String &address, int heading, bool pounce, float latitude, float longitude) {
  // Look for an existing record to update.
  for (int i = 0; i < deviceCount; i++) {
    if (devices[i].address == address) {
      devices[i].heading = heading;
      devices[i].pounce = pounce;
      devices[i].latitude = latitude;
      devices[i].longitude = longitude;
      devices[i].lastUpdate = millis();
      return;
    }
  }
  // Add a new record if there is available space.
  if (deviceCount < MAX_DEVICES) {
    devices[deviceCount].address = address;
    devices[deviceCount].heading = heading;
    devices[deviceCount].pounce = pounce;
    devices[deviceCount].latitude = latitude;
    devices[deviceCount].longitude = longitude;
    devices[deviceCount].lastUpdate = millis();
    deviceCount++;
  }
}

void BLEsupport::loop() 
{
  unsigned long currentMillis = millis();
  // Update our advertisement every UPDATE_INTERVAL.
  if (currentMillis - lastOwnUpdate >= UPDATE_INTERVAL) 
  {
    updateAdvertisement();
    lastOwnUpdate = currentMillis;
  }

  checkWatchdog();
}

