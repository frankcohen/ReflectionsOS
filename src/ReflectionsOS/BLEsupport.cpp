/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "BLEsupport.h"

// ----------------- SERVER CALLBACK IMPLEMENTATION -----------------

// Constructor for the nested MyCharacteristicCallbacks.
BLEsupport::MyCharacteristicCallbacks::MyCharacteristicCallbacks(BLEsupport* parent)
  : _parent(parent)
{
}

// Callback invoked when a client reads the characteristic.
void BLEsupport::MyCharacteristicCallbacks::onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
  Serial.println("Characteristic read requested");
  String json = _parent->getJsonData();
  Serial.print("Sending JSON: ");
  Serial.println(json);
  pCharacteristic->setValue((uint8_t*)json.c_str(), json.length());
}

// ----------------- CLIENT SCAN CALLBACK IMPLEMENTATION -----------------

// Constructor for the nested ScanCallbacks.
BLEsupport::ScanCallbacks::ScanCallbacks(BLEsupport* parent)
  : _parent(parent)
{
}

// onResult is called for each discovered advertised device.
void BLEsupport::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
  // Check if the advertised device contains our service UUID.
  if (!advertisedDevice->isAdvertisingService(BLE_SERVICE_UUID))
    return;
  
  String addr = String(advertisedDevice->getAddress().toString().c_str());

  String mef = F( "Found device with our service: " );
  mef += addr;
  Serial.println( mef );

  // If already known, update its lastUpdate timestamp.
  if (_parent->remoteDevices.find(addr) != _parent->remoteDevices.end()) {
    _parent->remoteDevices[addr].lastUpdate = millis();
    return;
  }
  // Otherwise, attempt to connect and read.
  _parent->connectAndRead(const_cast<NimBLEAdvertisedDevice*>(advertisedDevice));
}

// ----------------- BLEsupport METHODS -----------------

// The BLEsupport constructor uses an initializer list to initialize scanCallbacks.
BLEsupport::BLEsupport() : scanCallbacks(this) {
  compasstime = 0;
  lastAdvUpdate = 0;
  pServer = nullptr;
  pService = nullptr;
  pCharacteristic = nullptr;
  pAdvertising = nullptr;
}

void BLEsupport::begin() {  
  lastAdvUpdate = 0;
  
  gps.on();

  // Initialize the BLE device using the WiFi device name.
  NimBLEDevice::init(wifi.getDeviceName().c_str());
  
  // --- SERVER SETUP ---
  pServer = NimBLEDevice::createServer();
  pService = pServer->createService(BLE_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(BLE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ);
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks(this));
  pService->start();
  
  pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  
  NimBLEAdvertisementData scanResponse;
  scanResponse.setName(wifi.getDeviceName().c_str());
  pAdvertising->setScanResponseData(scanResponse);
  
  pAdvertising->start();
  Serial.println("BLE Advertising started");

  lastAdvUpdate = millis();
  
  // --- CLIENT SETUP ---

/*
  Serial.println("BLE client starting");
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&scanCallbacks, false);
  pScan->setActiveScan(true); // Enable active scanning.
  pScan->start(SCAN_TIME_MS, false);  // false: do not resume scanning automatically.
  Serial.println("BLE client started");
*/

    compasstime = millis();
}

String BLEsupport::getJsonData()
{
  Serial.print( "getJsonData() here ");
  Serial.println( compass.getHeading() );

  StaticJsonDocument<256> doc;
  doc["devicename"] = wifi.getDeviceName();
  doc["heading"] = compass.getHeading();
  doc["pounce"] = false;
  doc["latitude"] = gps.getLat();
  doc["longitude"] = gps.getLng();
  String output;
  serializeJson(doc, output);
  return output;
}

void BLEsupport::watchdogCheck() {
  return;   // Server only

  unsigned long currentMillis = millis();
  if (currentMillis - lastAdvUpdate > OWN_WATCHDOG_TIMEOUT) {
    lastAdvUpdate = currentMillis;

    // Stop advertising
    if ( pAdvertising ) pAdvertising->stop();
    
    // Stop any ongoing scan
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->stop();
    
    pScan->start( BLE_CLIENT_SCAN_TIME, false);
    
    // After scanning, resume advertising.
    if ( pAdvertising ) 
    {
      NimBLEAdvertisementData scanResponse;
      scanResponse.setName( wifi.getDeviceName().c_str() );
      pAdvertising->setScanResponseData(scanResponse);
      pAdvertising->start();
    }
  }
}

/* Client connects to a known service and gets its data */

void BLEsupport::connectAndRead(NimBLEAdvertisedDevice* advertisedDevice) 
{
  return;



  // Convert the advertised device address to an Arduino String.
  String addr = String( advertisedDevice->getAddress().toString().c_str() );
  
  // Static set to track connection attempts.
  static std::set<String> pendingConnections;
  
  // If a connection attempt is already in progress for this address, skip it.
  if (pendingConnections.find(addr) != pendingConnections.end()) 
  {
    String mef = F( "Already trying to connect to ");
    mef += addr;
    Serial.println( mef );
    return;
  }

  // Mark this address as pending.
  pendingConnections.insert(addr);
  
  // Check if there's already an active connection.
  std::vector<NimBLEClient*> clients = NimBLEDevice::getConnectedClients();
  for (auto client : clients) {
    if (client->getPeerAddress().equals(advertisedDevice->getAddress())) 
    {
      String mef = F("Already connected to ");
      mef += addr;
      Serial.println( mef );

      pendingConnections.erase(addr);
      return;
    }
  }
  
  String mef3 = F("Attempting to connect to ");
  mef3 += addr;
  Serial.println( mef3 );
  
  NimBLEClient* pClient = NimBLEDevice::createClient();

  if (!pClient->connect(advertisedDevice, BLE_CLIENT_ATTEMPT_TIME)) 
  { 
    String mef4 = F( "Client failed to connect to " );
    mef4 += addr;
    Serial.println( mef4 );

    // Attempt to force a disconnect up to 5 times (total about 20 seconds).
    const int maxAttempts = 5;
    const unsigned long disconnectInterval = 4000; // 4 seconds between attempts
    int attempts = 0;

    while ( ( pClient->isConnected() ) && ( attempts < maxAttempts ) ) 
    {
      String mef = F("Client appears connected; attempting disconnect...");
      mef += attempts;
      Serial.println( mef );

      pClient->disconnect();
      
      delay(disconnectInterval);
      attempts++;
    }
    
    NimBLEDevice::deleteClient(pClient);
    pendingConnections.erase(addr);
    return;
  }

  Serial.println( F( "Connected to remote device!" ) );
  
  // Get the remote service.
  NimBLERemoteService* pRemoteService = pClient->getService( BLE_SERVICE_UUID );
  if (!pRemoteService) 
  {
    Serial.println( F( "Remote service not found." ) );
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    pendingConnections.erase(addr);
    return;
  }
  
  // Get the remote characteristic.
  NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(BLE_CHARACTERISTIC_UUID);
  if (!pRemoteCharacteristic) {
    Serial.println( F( "Remote characteristic not found." ) );
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    pendingConnections.erase(addr);
    return;
  }
  
  // Read the JSON value (readValue() returns an std::string).
  std::string value = pRemoteCharacteristic->readValue();
  String json = String(value.c_str());
  String mef2 = F( "Read JSON from remote: " );
  mef2 += json;
  Serial.println( mef2 );
  
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) 
  {
    String mef = F( "JSON parse failed: " );
    mef += err.c_str();
    Serial.println( mef );
  } 
  else 
  {
    ReflectionsData data;
    data.devicename = String(wifi.getDeviceName().c_str());
    data.heading = doc[ F( "heading" ) ] | 0;
    data.pounce = doc[ F( "pounce" ) ] | false;
    data.latitude = doc[ F( "latitude" ) ] | 0.0;
    data.longitude = doc[ F( "longitude" ) ] | 0.0;
    
    // Enforce a maximum number of remote devices.
    if (remoteDevices.size() >= MAX_DEVICES) {
      auto oldest = remoteDevices.begin();
      for (auto it = remoteDevices.begin(); it != remoteDevices.end(); ++it) {
        if (it->second.lastUpdate < oldest->second.lastUpdate)
          oldest = it;
      }
      String mef = F( "Max devices reached. Removing oldest device: " );
      mef += oldest->first;
      Serial.println( mef );

      remoteDevices.erase(oldest);
    }
    data.lastUpdate = millis();
    remoteDevices[addr] = data;
    Serial.println( F( "Stored remote device data." ) );
  }
  
  // Proper cleanup: disconnect and delete the client.
  pClient->disconnect();
  NimBLEDevice::deleteClient(pClient);
  
  // Remove the address from pending connections.
  pendingConnections.erase(addr);
}

void BLEsupport::loop() {
  watchdogCheck();
  
  if (millis() - compasstime > 5000) {
    compasstime = millis();
    printRemoteDevices();
    //Serial.print("Compass Heading: ");
    //Serial.println(compass.getHeading());
  }
}

void BLEsupport::printRemoteDevices() 
{
  if (remoteDevices.empty()) return;

  Serial.println("Remote Devices:");

  for (auto const &entry : remoteDevices) {
    Serial.print("Address: ");
    Serial.println(entry.first);
    Serial.print("Device Name: ");
    Serial.println(entry.second.devicename);
    Serial.print("Heading: ");
    Serial.println(entry.second.heading);
    Serial.print("Latitude: ");
    Serial.println(entry.second.latitude);
    Serial.print("Longitude: ");
    Serial.println(entry.second.longitude);
    Serial.print("Last update (ms): ");
    Serial.println(entry.second.lastUpdate);
  }
}
