/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "BLEsupport.h"

// Declare the ClientCallbacks instance here
BLEsupport::ClientCallbacks clientCallbacks; 

// ----------------- SERVER CALLBACK IMPLEMENTATION -----------------

// Constructor for the nested MyCharacteristicCallbacks.
BLEsupport::MyCharacteristicCallbacks::MyCharacteristicCallbacks(BLEsupport* parent)
  : _parent(parent)
{
}

// Callback invoked when a client reads the characteristic.
void BLEsupport::MyCharacteristicCallbacks::onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
  Serial.println(F("Characteristic read requested"));
  String json = _parent->getJsonData();
  Serial.print(F("Sending JSON: "));
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
void BLEsupport::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice)
{
  //Serial.print(F("Discovered device: "));
  //Serial.println(advertisedDevice->getAddress().toString().c_str());

  // Skip self advertisement
  if ( advertisedDevice->getAddress().equals( NimBLEDevice::getAddress() ) ) 
  {
    Serial.println(F("Skipping self advertisement"));
    return;
  }

  // Only process devices advertising our service UUID.
  if ( !advertisedDevice->isAdvertisingService( NimBLEUUID( BLE_SERVICE_UUID ) ) ) 
  {
    //Serial.println( F("Doesn't match our service") );
    return;
  }
    
  // NOTE: Removed the stop() call to allow continuous scanning.
  // NimBLEDevice::getScan()->stop();

  // Save the device reference for potential connection usage.
  advDevice = advertisedDevice;
  doConnect = true;  // Set flag to trigger connection logic elsewhere.

  // Debug: Print device address.
  String deviceAddress = String(advertisedDevice->getAddress().toString().c_str());

  /*
  Serial.print(F("Received advertisement from device: "));
  Serial.println(deviceAddress);
  */

  // Add or update the device data in remoteDevices.
  ReflectionsData& data = _parent->remoteDevices[deviceAddress];
  
  data.devicename = String(advertisedDevice->getName().c_str());
  data.lastUpdate = millis();

  float heading = compass.getHeading();
  if ((heading > 0) && (heading < 360)) {
    data.heading = (int)heading;
  } else {
    data.heading = 0;  // Use default value if heading is invalid.
  }

  data.rssi = advertisedDevice->getRSSI();
  data.pounce = mypounce;  // Set pounce state as needed.
  data.latitude = gps.getLat();
  data.longitude = gps.getLng();

  // Optionally, reset the connection flag after processing (if not needed to trigger a connection every time).
  // doConnect = false;
}

void BLEsupport::ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) 
{
  Serial.printf(F("Scan Ended, reason: %d, device count: %d; Restarting scan\n"), reason, results.getCount());
  // Restart scanning automatically.
  NimBLEDevice::getScan()->start(scanTimeMs, false, true);
}

/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) 
{
    std::string str  = (isNotify ? "Notification" : "Indication");
    str += " from ";
    str += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
    str += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    str += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
    str += ", Value = " + std::string((char*)pData, length);
    Serial.printf(F("%s\n"), str.c_str());
}

// ----------------- BLEsupport METHODS -----------------

BLEsupport::BLEsupport() : scanCallbacks(this) {
  compasstime = 0;
  lastAdvUpdate = 0;
  lastPrintTime = 0;  // Initialize the last print time.
  pServer = nullptr;
  pService = nullptr;
  pCharacteristic = nullptr;
  pAdvertising = nullptr;
  lastServerUpdate = 0;
}

void BLEsupport::begin() {  
  lastAdvUpdate = 0;
  mypounce = false;  
  pnctime = millis();

Serial.println("1");
  gps.on();

Serial.println("1a");

  // Initialize the BLE device using the WiFi device name.
  NimBLEDevice::init(wifi.getDeviceName().c_str());
  NimBLEDevice::setPower(3);  // 3dbm
Serial.println("1b");

  // Server Setup.
  pServer = NimBLEDevice::createServer();
  pService = pServer->createService(BLE_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(BLE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ);
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks(this));
  pService->start();
  Serial.println("1c");

  pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
Serial.println("1d");

  // Set both the maximum and minimum interval to the same value for a consistent advertisement rate
  pAdvertising->setMinInterval(advInterval);
  pAdvertising->setMaxInterval(advInterval);
  Serial.println("1e");

  NimBLEAdvertisementData scanResponse;
  scanResponse.setName(wifi.getDeviceName().c_str());
  pAdvertising->setScanResponseData(scanResponse);
  pAdvertising->start();

  lastAdvUpdate = millis();
Serial.println("2");

  // Client Setup.
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&scanCallbacks, false);

  // Set scan interval and window in milliseconds.
  pScan->setInterval( scanInterval );
  pScan->setWindow( scanWindow );

  pScan->setActiveScan(true); // Enable active scanning.
  pScan->setDuplicateFilter(false);

  // Start scanning continuously. The scan will not automatically resume after stopping.
  pScan->start(SCAN_TIME_MS, false);

  Serial.println(F("BLE started"));

Serial.println("3");

  compasstime = millis();
}

void BLEsupport::setPounce( bool pnc )
{
  mypounce = pnc;
  pnctime = millis();
}

bool BLEsupport::getPounce()
{
  return mypounce;
}

String BLEsupport::getJsonData()
{
  StaticJsonDocument<256> doc;
  doc["devicename"] = wifi.getDeviceName();

  float heading = compass.getHeading();
  if ((heading > 0) && (heading < 360)) {
    doc["heading"] = (int)heading;
  } else {
    doc["heading"] = 0;
  }

  doc["pounce"] = mypounce;
  doc["latitude"] = gps.getLat();
  doc["longitude"] = gps.getLng();
  String output;
  serializeJson(doc, output);
  return output;
}

int BLEsupport::getRemoteDevicesCount() 
{
  return remoteDevices.size();
}
	
bool BLEsupport::isAnyDevicePounceTrue() {
  // Iterate over the remote devices to check if any has a true pounce value
  for (const auto& entry : remoteDevices) {
    if (entry.second.pounce) {
      return true;  // Return true if any device has pounce set to true
    }
  }
  return false;  // Return false if no device has pounce set to true
}

void BLEsupport::printRemoteDevices() 
{
  bool first = true;

  for (const auto& entry : remoteDevices) 
  {
    if ( first )
    {
      Serial.println(F("Tracking Remote Devices:"));
      first = false;
    }

    const ReflectionsData& data = entry.second;
    Serial.print(F("  Device Name: "));
    Serial.print(data.devicename);
    Serial.print(F(", Heading: "));
    Serial.print(data.heading);
    Serial.print(F(", Pounce: "));
    Serial.print(data.pounce);
    Serial.print(F(", Latitude: "));
    Serial.print(data.latitude);
    Serial.print(F(", Longitude: "));
    Serial.print(data.longitude);
    Serial.print(F(", RSSI: "));
    Serial.println(data.rssi);  
  }

  if ( first )
  {
    Serial.println( F("No remote devices tracked") );
  }

}

void BLEsupport::loop()
{
  unsigned long currentMillis = millis();
  
  // Print remote devices' data every 30 seconds.
  if (currentMillis - lastPrintTime >= 30000) {
    printRemoteDevices();
    lastPrintTime = currentMillis;
  }

  // Remove stale remote devices (timeout after 2 minutes).
  for ( auto it = remoteDevices.begin(); it != remoteDevices.end(); ) 
  {
    if ( currentMillis - it->second.lastUpdate >= 60000 ) 
    {
      Serial.print("Removing stale device: ");
      Serial.println(it->first);
      it = remoteDevices.erase(it);
    } 
    else 
    {
      ++it;
    }
  }
  
  // Update the server's characteristic value every 20 seconds.
  if (currentMillis - lastServerUpdate >= 20000) {
    if (pCharacteristic != nullptr) {
      String json = getJsonData();
      pCharacteristic->setValue((uint8_t*)json.c_str(), json.length());
      Serial.print("Server updated: ");
      Serial.println( json );
    }
    lastServerUpdate = currentMillis;
  }

  if ( millis() - pnctime > 30000 )
  {
    pnctime = millis();
    mypounce = false;
  }

}

