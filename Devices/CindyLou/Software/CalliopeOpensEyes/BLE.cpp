/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 I originally tried using the ESP32 blue BLE library. I ran out of memory. Switched to
 NimBLE-Arduino, an Arduino fork of Apache NimBLE, a replacement open-source BLE library
 https://github.com/h2zero/NimBLE-Arduino
 https://www.arduino.cc/reference/en/libraries/nimble-arduino/
 https://h2zero.github.io/esp-nimble-cpp/md__migration_guide.html#autotoc_md46

Todo:
Make this work among multiple Reflections devices
Balance the active scanning for battery life

*/

#include "BLE.h"

BLE::BLE(){}

// Server

static NimBLEServer* pServer;
static boolean serverConnected;
static boolean clientConnected;

// Client

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// The remote service we wish to connect to.
static BLEUUID serviceUUID( SERVICE_UUID_CALLIOPE );
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID( CHARACTERISTIC_UUID_HEADING );

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("BLE server connected");
        NimBLEDevice::startAdvertising();
    };

    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("BLE server says Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
        Serial.printf("BLE MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };
};

/** Handler class for server characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        Serial.print("BLE server ");
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };
    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };

    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", ";
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};

/** Handler class for server descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
        std::string dscVal = pDescriptor->getValue();
        Serial.print("Descriptor witten value:");
        Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
        Serial.print(pDescriptor->getUUID().toString().c_str());
        Serial.println(" Descriptor read");
    };
};

// Client callbbacks

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }

  // Security methods go here
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic( charUUID );
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println( charUUID.toString().c_str() );
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 * Called for each advertising BLE server.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
   
  void onResult(BLEAdvertisedDevice* advertisedDevice) {

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService( serviceUUID ) ) {

      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice->toString().c_str());

      BLEDevice::getScan()->stop();
      myDevice = advertisedDevice;
      doConnect = true;
      doScan = false;
    }
  }
};

static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;
static NimBLECharacteristic* pHeadingCharacteristic;

void BLE::begin()
{
  std::string devname = "CALLIOPE-";
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  NimBLEDevice::init( devname );

  Serial.print("BLE begin for ");
  Serial.println( devname.c_str() );

  clientWaitTime = millis();
  clientConnected = false;
  serverWaitTime = millis();
  serverConnected = false;

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pCalliopeService = pServer->createService( SERVICE_UUID_CALLIOPE );

  pHeadingCharacteristic = pCalliopeService->createCharacteristic
  (
    CHARACTERISTIC_UUID_HEADING,
    NIMBLE_PROPERTY::READ
  );

  pHeadingCharacteristic->setCallbacks(&chrCallbacks);

  pCalliopeService -> start();

  Serial.println( F( "BLE server started" ) );

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising -> addServiceUUID( pCalliopeService -> getUUID() );
  pAdvertising -> start();

  serverConnected = true;

  Serial.println("BLE server advertising Started");

/*
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  Serial.println("BLE client started");
*/

}

String BLE::getHeading()
{
  return heading;
}

void BLE::setHeading( String _heading )
{
  heading = _heading;
}

void BLE::loop()
{  
  // Server sending direction heading

  if ( serverConnected )
  {
    if ((millis() - serverWaitTime) > 5000) {
      serverWaitTime = millis();

      std::string mys = heading.c_str();
      pHeadingCharacteristic->setValue( mys );
      //pHeadingCharacteristic->notify();
    }
  }
  else
  {
    if ((millis() - serverWaitTime) > 5000) {
      serverWaitTime = millis();

      pServer->startAdvertising(); // restart advertising
      Serial.println("Started advertising");
    }
  }
}
