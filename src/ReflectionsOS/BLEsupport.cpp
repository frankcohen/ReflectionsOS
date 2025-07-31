/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Supports inter-watch communication over Bluetooth Low Energy (BLE)

 Note:
 NIMble and PNGDEC libraries have a conflict. PNGDEC defines
 local as a macro for static. This causes an "unqualified-id before static"
 compiler error. In libraries/PNGdec/src/zutil.h line 38 
 changed #define local static to #define PNGDEC_LOCAL static
 and changed its use in zutil.c on line 200 and 207 and
 alder32.c line 140

*/

#include "BLEsupport.h"

// Constructor for the BLEsupport class
BLEsupport::BLEsupport() {}

void BLEsupport::begin() 
{    
    // Initialize the BLE device
    NimBLEDevice::init( wifi.getDeviceName().c_str() );
    NimBLEDevice::setPower(3);  // 3dbm

    setupServer();

    // Start advertising
    pAdvertising->start();

    mypounce = false;
    pnctime = millis();

    Serial.println("BLE started");
}

void BLEsupport::setupServer() 
{
    // Create a new BLE server
    pServer = NimBLEDevice::createServer();

    // Create a BLE service
    pService = pServer->createService( BLE_SERVER_UUID );

    // Create a BLE characteristic
    pCharacteristic = pService->createCharacteristic(BLE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ );
    pCharacteristic->setValue("Initial Data");

    // Start the service
    pService->start();

    // Create advertising
    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID( BLE_SERVER_UUID );

    NimBLEAdvertisementData scanResponse;
    scanResponse.setName( wifi.getDeviceName().c_str() );
    pAdvertising->setScanResponseData( scanResponse );

    pAdvertising->start();
}

void BLEsupport::handleBLEConnections() 
{
    if (pServer == nullptr) {
        Serial.println("BLE pServer is not initialized.");
        return;
    }

    // Check if the server has connected clients
    if (pServer->getConnectedCount() > 0) {
        // Iterate through each connected client
        for (int i = 0; i < pServer->getConnectedCount(); i++) {
            NimBLEClient* pClient = pServer->getClient(i);

            // Check if the client is advertising the required service and characteristic UUID
            if (pClient->isConnected()) 
            {
                Serial.println(" isConnected");
                NimBLERemoteService* pService = pClient->getService(NimBLEUUID(BLE_SERVER_UUID));
                if (pService) {
                    NimBLERemoteCharacteristic* pChar = pService->getCharacteristic(NimBLEUUID(BLE_CHARACTERISTIC_UUID));
                    if (pChar) {
                        // Get the client's MAC address
                        std::string deviceAddress = pClient->getPeerAddress().toString();

                Serial.print("Found ");
                Serial.println( pClient->getPeerAddress() );

                        // Check if this device address is already in the list of unique devices
                        auto it = std::find(uniqueDeviceAddresses.begin(), uniqueDeviceAddresses.end(), deviceAddress);
                        if (it == uniqueDeviceAddresses.end()) {
                            // If the device is not in the list, add it
                            uniqueDeviceAddresses.push_back(deviceAddress);
                        }
                    }
                }
            }
        }
    }
}

void BLEsupport::sendJsonData(const JsonDocument& doc) {
    // Convert JSON data to a string
    String jsonString;
    serializeJson(doc, jsonString);

    // Send the JSON string to the client through the characteristic
    pCharacteristic->setValue(jsonString.c_str());
    pCharacteristic->notify();  // Notify connected clients about the new data
}

bool BLEsupport::readJsonData(JsonDocument& doc) {
    // Read the data from the characteristic (incoming data from clients)
    String jsonString = pCharacteristic->getValue().c_str();

    // Parse the JSON string into the provided JSON document
    DeserializationError error = deserializeJson(doc, jsonString);

    // Return whether the JSON data was successfully parsed
    return !error;
}

void BLEsupport::setJsonData(const String& devicename, float heading, bool pounce, float latitude, float longitude) {
    // Set the JSON fields with the provided values
    jsonData["devicename"] = devicename;
    jsonData["heading"] = heading;
    jsonData["pounce"] = pounce;
    jsonData["latitude"] = latitude;
    jsonData["longitude"] = longitude;
}

void BLEsupport::scanForDevices() {
    // Start scanning for nearby BLE devices
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);  // Active scanning to receive more detailed results
    pScan->setInterval(1349);    // Scan interval (in 0.625 ms units)
    pScan->setWindow(449);       // Scan window (in 0.625 ms units)
    pScan->start(5, false);      // Scan for 5 seconds, non-blocking
}

void BLEsupport::printRemoteDevices() {
    // Start scanning for remote devices
    scanForDevices();

    // Wait for the scan to complete
    delay(5000);  // Wait for the scanning process to finish

    // Check the scan results for devices matching the BLE_SERVER_UUID and BLE_CHARACTERISTIC_UUID
    NimBLEScanResults results = NimBLEDevice::getScan()->getResults();
    for (int i = 0; i < results.getCount(); i++) {
        const NimBLEAdvertisedDevice* advertisedDevice = results.getDevice(i);

        // Check if the device advertises the required service UUID
        if (advertisedDevice->isAdvertisingService(NimBLEUUID(BLE_SERVER_UUID))) {
            Serial.print("Found device: ");
            Serial.println(advertisedDevice->getAddress().toString().c_str());
            
            // Try to connect to the device and check if it has the required characteristic
            NimBLEClient* pClient = NimBLEDevice::createClient();
            pClient->connect(advertisedDevice);
            if (pClient->isConnected()) {
                NimBLERemoteService* pService = pClient->getService(NimBLEUUID(BLE_SERVER_UUID));
                if (pService) {
                    NimBLERemoteCharacteristic* pChar = pService->getCharacteristic(NimBLEUUID(BLE_CHARACTERISTIC_UUID));
                    if (pChar) {
                        Serial.println("  Found matching characteristic!");
                    } else {
                        Serial.println("  Characteristic not found");
                    }
                }
                pClient->disconnect();
            } else {
                Serial.println("  Failed to connect to the device");
            }
        }
    }
}

int BLEsupport::getRemoteDevicesCount() {
    return uniqueDeviceAddresses.size();  // Return the size of the list of unique device addresses
}

bool BLEsupport::isAnyDevicePounceTrue() {
    unsigned long currentTime = millis();
    bool pounceDetected = false;

    // Check if any device's pounce is true within the last 10 seconds
    for (const PounceData& data : pounceDataList) {
        if (data.pounce && (currentTime - data.timestamp <= 10000)) {  // 10 seconds threshold
            pounceDetected = true;
            break;  // No need to check further once a valid pounce is found
        }
    }

    return pounceDetected;
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

void BLEsupport::loop() {
    //handleBLEConnections();
}

