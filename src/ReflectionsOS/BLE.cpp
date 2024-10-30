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

#include "BLE.h"

extern LOGGER logger;
extern Compass compass;

static int latestheading;
static bool gotAPounce;
static unsigned long pounceTimer;

extern BLEServerClass bleServer;
extern BLEClientClass bleClient;

BLEServerClass::BLEServerClass() : pServer(nullptr), pCharacteristic(nullptr), pClient(nullptr), pRemoteCharacteristic(nullptr) {}

void BLEServerClass::begin() 
{
  gotAPounce = false;
  pounceTimer = millis();

  uint8_t mac[6];
  char name[32];
  esp_read_mac(mac, ESP_MAC_BT);
  sprintf(name, "Reflections-S-%02X%02X", mac[4], mac[5]);

  Serial.println ( name );

  BLEDevice::init( name );

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  pService = pServer->createService( MY_SERVICE_UUID );

  /*
  BLECharacteristic::PROPERTY_READ
  BLECharacteristic::PROPERTY_WRITE
  */

  pCharacteristic = pService->createCharacteristic(
      MY_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  // Initialize GAP callback
  // esp_ble_gap_register_callback(gapEventHandler);

  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID( MY_SERVICE_UUID );
  pAdvertising->setScanResponse( true );
  pAdvertising->setMinPreferred(0x06);  // Set min and max advertising interval
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->start();
  
  Serial.println( "Server started" );
}

bool BLEServerClass::isPounced()
{
  bool mep = gotAPounce;
  gotAPounce = false;
  return mep;
}

void BLEServerClass::handleClientData() {
    // Optionally handle other operations here
}

void BLEServerClass::sendJsonData(String jsonData) {
    if (pCharacteristic) {
        pCharacteristic->setValue(jsonData.c_str());
        pCharacteristic->notify();
    }
}

void BLEServerClass::MyServerCallbacks::onConnect(BLEServer* pServer) {
    Serial.println("Server says client connected");
}

void BLEServerClass::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("Server says client disconnected");
}

void BLEServerClass::setLatestHeading( int myh )
{
  latestheading = myh;
}

int BLEServerClass::getLatestHeading()
{
  return latestheading;
}

void BLEServerClass::MyCharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    Serial.print( "getValue = " );
    Serial.println( pCharacteristic->getValue().c_str() );

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, value.c_str());

    if (doc.containsKey("heading")) 
    {
        latestheading = doc[ "heading" ];
        //Serial.print("Heading: " );
        //Serial.println( latestheading );
    }
    
    else if (doc.containsKey("pounce")) 
    {
      int pounce = doc["pounce"];
      Serial.println("Pounce: " + String(pounce));

      if ( millis() - pounceTimer > 5000 )
      {
        pounceTimer = millis();
        gotAPounce = true;      
      }

    } 
    
    else if (doc.containsKey("location")) 
    {
        float longitude = doc["location"]["longitude"];
        float latitude = doc["location"]["latitude"];
        String time = doc["location"]["time"].as<String>();
        Serial.println("Location - Longitude: " + String(longitude) + ", Latitude: " + String(latitude) + ", Time: " + time);
    }
}

BLEClientClass::BLEClientClass() : pClient(nullptr), pRemoteCharacteristic(nullptr) {}



void BLEClientClass::begin() 
{
  sendHeadingTimer = millis();
  checkRSSItimer = millis();
  clientReconnectTimer = millis();

  uint8_t mac[6];
  char name[32];
  esp_read_mac(mac, ESP_MAC_BT);
  sprintf(name, "Reflections-C-%02X%02X", mac[4], mac[5]);

  BLEDevice::init( name );

  clientConnect();
}

void BLEClientClass::clientConnect()
{
  if ( pClient != nullptr )
  {
    if ( pClient->isConnected() ) return;
  }

  BLEScan* pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);
  BLEScanResults results = pScan->start(5);

  // Iterate through the discovered devices
  for (int i = 0; i < results.getCount(); i++) 
  {
    BLEAdvertisedDevice device = results.getDevice(i);
    
    // Print device details
    Serial.print("Device ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print("Address: ");
    Serial.print(device.getAddress().toString().c_str());
    Serial.print(", Name: ");
    Serial.print(device.getName().c_str());

    // Print service UUIDs
    Serial.print(", Advertised Service UUIDs: ");
    if (device.haveServiceUUID()) {
        BLEUUID serviceUUID = device.getServiceUUID();
        Serial.print(serviceUUID.toString().c_str());
    } else {
        Serial.print("None");
    }
    Serial.println();
    
    if (device.haveServiceUUID() && device.isAdvertisingService( BLEUUID( MY_SERVICE_UUID ) ) ) 
    {
      // Create a new BLEClient instance
      pClient = BLEDevice::createClient();

      // Connect to the server using a pointer to the advertised device
      pClient->connect(&device);  // Pass pointer to BLEAdvertisedDevice

      if ( pClient ->isConnected() )
      {
        Serial.print( "Client connected, RSSI = " );
        Serial.println( pClient -> getRssi() );
      }
      else
      {
        Serial.println( "Client not connected" );
      }

      // Get the remote service and characteristic
      BLERemoteService* pRemoteService = pClient->getService( MY_SERVICE_UUID );
      if ( pRemoteService ) 
      {
        pRemoteCharacteristic = pRemoteService->getCharacteristic( MY_CHARACTERISTIC_UUID );
        if ( pRemoteCharacteristic )
        {
          Serial.println( "pRemoteCharacteristic found" );
        }
        else
        {
          Serial.println( "pRemoteCharacteristic not found" );
        }
      }
      else
      {
        Serial.println( "pRemoteService not found" );
      }
    }
  }
}

/* Gets the RSSI signal strength / distance to the other device */

int BLEClientClass::getDistance() 
{
  int mei = latestrssi;
  latestrssi = 0;
  return mei;
}

void BLEClientClass::handleClientData() 
{
  if ( pClient != nullptr )
  {
    if ( ! pClient->isConnected() )
    {
      latestrssi = 0;
    }
    else
    {
      if ( millis() - checkRSSItimer > 2000 )
      {
        checkRSSItimer = millis();
        latestrssi = pClient->getRssi();
        Serial.print( "handleClientData RSSI = " );
        Serial.println( latestrssi );
      }
    }
  }

}

void BLEClientClass::sendJsonData(String jsonData) {
    if (pRemoteCharacteristic) {
        pRemoteCharacteristic->writeValue(jsonData.c_str());
    }
}

void BLEClientClass::MyClientCallbacks::onConnect(BLEClient* pClient) {
    Serial.println("Connected to server");
}

void BLEClientClass::MyClientCallbacks::onDisconnect(BLEClient* pClient) {
    Serial.println("Disconnected from server");
}

void BLEClientClass::sendPounce()
{
  String jsonData = "{\"pounce\":\"1\"";
  jsonData += ",\"time\":\"12:15:01 PST\"}";      // TODO: Use the ESP32 built-in RTC to provide the time
  bleClient.sendJsonData(jsonData);
  Serial.print( "Sending Pounce to server: ");
  Serial.println( jsonData );
}

void BLEClientClass::loop()
{
  if ( millis() - clientReconnectTimer > 15000 )
  {
    if ( pClient == nullptr )
    {
      Serial.println("Reconnecting - pClient null");
      clientConnect();
    }
    else
    {
      if ( ! pClient->isConnected() )
      {
        Serial.println("Reconnecting");
        clientConnect();
      }
    }
  }

  handleClientData();

  if ( millis() - sendHeadingTimer > 2000 )
  {
    sendHeadingTimer = millis();

    // Example of sending JSON data from the client
    String jsonData = "{\"heading\":";
    jsonData += String( compass.getHeading() );
    jsonData += ",\"time\":\"12:15:01 PST\"}";      // TODO: Use the ESP32 built-in RTC to provide the time
    bleClient.sendJsonData(jsonData);
    //Serial.print( "Sending compas heading to server: ");
    //Serial.println( jsonData );
  }

}
