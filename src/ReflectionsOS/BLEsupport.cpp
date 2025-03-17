/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

Todo:
Enable start-up to run in a thread
After the client and server connect the advertised client machine's server no long advertises its service
Make this work among multiple Reflections devices
Balance the active scanning for battery life

*/

#include "BLEsupport.h"

BLEsupport::BLEsupport(){}

void BLEsupport::begin() 
{
  Serial.println("Initializing BLE...");
  
  deviceCount = 0;
  lastOwnUpdate = 0;
  pAdvertising = nullptr;
  pBLEScan = nullptr;

  String devname = host_name_me;
  String mac = WiFi.macAddress().c_str();
  devname.concat( mac.substring( 15, 17 ) );
  devicename = devname.c_str();

  BLEDevice::init( devicename );

  pAdvertising = BLEDevice::getAdvertising();
  BLEAdvertisementData advData;
  advData.setName("ESP32_BLE");

  // Prepare initial advertisement JSON containing heading, pounce, and GPS location.
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

  advData.setServiceData(BLEUUID( BLE_SERVICE_UUID ), jsonData.c_str());

  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  Serial.println("BLE Advertising started");

  // --- Setup BLE Scanning ---
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
  pBLEScan->setActiveScan(true);
  // Duration 0 means scanning runs continuously.
  pBLEScan->start(0, nullptr, false);

  lastOwnUpdate = millis();
}

void BLEsupport::loop() 
{
  unsigned long currentMillis = millis();
  // Update our advertisement every UPDATE_INTERVAL.
  if (currentMillis - lastOwnUpdate >= UPDATE_INTERVAL) {
    updateAdvertisement();
    lastOwnUpdate = currentMillis;
  }
  checkWatchdog();
  delay(100);
}

void BLEsupport::updateAdvertisement() 
{
  /*
  int heading = getCompassHeading();
  bool pounce = getPounce();
  float latitude = getLatitude();
  float longitude = getLongitude();
  Serial.print("Updating advertisement - Heading: ");
  Serial.print(heading);
  Serial.print(", Pounce: ");
  Serial.print(pounce ? "true" : "false");
  Serial.print(", Latitude: ");
  Serial.print(latitude);
  Serial.print(", Longitude: ");
  Serial.println(longitude);
  */

  // Prepare initial advertisement JSON containing heading, pounce, and GPS location.
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

  BLEAdvertisementData advData;
  advData.setName("ESP32_BLE");
  advData.setServiceData( BLEUUID( BLE_SERVICE_UUID ), jsonData.c_str());

  pAdvertising->stop();  // Stop advertising to update data.
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->start();
}

void BLEsupport::checkWatchdog() 
{
  unsigned long currentMillis = millis();
  
  // --- Own Advertisement Watchdog ---
  if (currentMillis - lastOwnUpdate >= OWN_WATCHDOG_TIMEOUT) 
  {
    Serial.println("Warning: Own advertisement not updated within expected interval!");
  }

  // --- Remote Devices Watchdog ---
  for (int i = 0; i < deviceCount; i++) 
  {
    if (currentMillis - devices[i].lastUpdate >= REMOTE_WATCHDOG_TIMEOUT) 
    {
      Serial.print("Warning: Lost updates from device ");
      Serial.print(devices[i].address);
      Serial.println(" - no update received recently!");
      // Remove the stale device entry by shifting remaining elements.
      for (int j = i; j < deviceCount - 1; j++) 
      {
        devices[j] = devices[j + 1];
      }
      deviceCount--;
      i--;  // Adjust index after removal.
    }
  }
}

void BLEsupport::updateDevice(const String& address, int heading, bool pounce, float latitude, float longitude) 
{
  // Look for an existing record to update.
  for (int i = 0; i < deviceCount; i++) 
  {
    if (devices[i].address == address) 
    {
      devices[i].heading = heading;
      devices[i].pounce = pounce;
      devices[i].latitude = latitude;
      devices[i].longitude = longitude;
      devices[i].lastUpdate = millis();
      return;
    }
  }
  // Add a new record if there is available space.
  if (deviceCount < MAX_DEVICES) 
  {
    devices[deviceCount].address = address;
    devices[deviceCount].heading = heading;
    devices[deviceCount].pounce = pounce;
    devices[deviceCount].latitude = latitude;
    devices[deviceCount].longitude = longitude;
    devices[deviceCount].lastUpdate = millis();
    deviceCount++;
  }
}

// ----- Nested Callback Class Implementation -----

void BLEsupport::MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) 
{
  if (advertisedDevice.haveServiceData() &&
      advertisedDevice.isAdvertisingService(BLEUUID( BLE_SERVICE_UUID ))) 
  {
    String serviceData = advertisedDevice.getServiceData();

    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, serviceData.c_str() );
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
    String devAddress = advertisedDevice.getAddress().toString().c_str();

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

    parentService->updateDevice(devAddress, receivedHeading, receivedPounce, receivedLat, receivedLon);
  }
}
