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
 See https://github.com/bitbank2/PNGdec/issues/36

*/

#include "BLEsupport.h"

static const NimBLEAdvertisedDevice* advDevice;
static bool doConnect;
static NimBLEServer* pServer;

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override 
    { 
        //Serial.printf( "Connected\n"); 
    }

    void onDisconnect(NimBLEClient* pClient, int reason) override 
    {
        //Serial.printf("%s Disconnected, reason = %d\n", pClient->getPeerAddress().toString().c_str(), reason);
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} clientCallbacks;

/** Callbacks when scan events are received */

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override 
    {
        if (advertisedDevice->isAdvertisingService(NimBLEUUID( BLE_SERVER_UUID ) ) )
        {
            //Serial.printf("Found: %s\n", advertisedDevice->toString().c_str());

            // TODO: Support messaging to multiple servers

            NimBLEDevice::getScan()->stop();
            advDevice = advertisedDevice;
            doConnect = true;
        }
    }

    /** Callback to process the results of the completed scan or restart it */
    void onScanEnd(const NimBLEScanResults& results, int reason) override 
    {
        //Serial.printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start( 5000, false, true);
    }
} scanCallbacks;

// Client
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override 
    {
        //Serial.printf("%s : onRead(), value: %s\n", pCharacteristic->getUUID().toString().c_str(), pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        //Serial.printf("%s : onWrite(), value: %s\n", pCharacteristic->getUUID().toString().c_str(), pCharacteristic->getValue().c_str());
    }
} chrCallbacks;

class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo ) override 
    {
        //Note: Disabled, NIMble says don't advertise while serving a connection
        //      later this will support multiple connections.

        //Serial.println("Client Connected. Restarting advertising.");
        // Restart advertising after a connection is made
        //NimBLEDevice::getAdvertising()->start();
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason ) override {
        //Serial.println("Client Disconnected. Restarting advertising.");
        // Restart advertising after a disconnect
        NimBLEDevice::getAdvertising()->start();
    }
} srvCallbacks;

// Server
class CharacteristicCallbacksServer : public NimBLECharacteristicCallbacks 
{
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        //Serial.printf("%s : onRead(), value: %s\n", pCharacteristic->getUUID().toString().c_str(), pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        //Serial.printf("%s : onWrite(), value: %s\n", pCharacteristic->getUUID().toString().c_str(), pCharacteristic->getValue().c_str() );

        String input = pCharacteristic->getValue().c_str();

        int equalsPos = input.indexOf( ':' );

        if (equalsPos != -1) 
        {
            // Extract the name (before the equals sign)
            String pnc = input.substring(0, equalsPos);
            if ( pnc == "true" ) s_pounce = true;
            if ( pnc == "false" ) s_pounce = false;
            
            // Extract the value (after the equals sign)
            String hdg = input.substring(equalsPos + 1);

            s_heading = hdg.toInt();
            s_when = millis();

            // Get the client associated with this connection using the correct getClient() method
            NimBLEServer* pServer = pCharacteristic->getService()->getServer();
            NimBLEClient* pClient = pServer->getClient(connInfo);  // Use getClient with connInfo

            if (pClient) {
                s_rssi = pClient->getRssi();  // Fetch RSSI from the client
            }

            // Print the parsed values
            Serial.print("Received Pounce: ");
            Serial.println( s_pounce );
            Serial.print("Heading: ");
            Serial.println( s_heading );
            Serial.print("RSSI: ");
            Serial.println( s_rssi );            
        } else {
            Serial.print("Data not valid: ");
            Serial.println( input );
        }

    }
} chrCallbacksServer;

BLEsupport::BLEsupport() {}

void BLEsupport::begin() 
{
    NimBLEDevice::init( wifi.getDeviceName().c_str() );
    NimBLEDevice::setPower(3);  // 3dbm

    setupServer();

    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(&scanCallbacks, false);
    pScan->setInterval(100);
    pScan->setWindow(100);
    pScan->setActiveScan(true);
    pScan->start( 5000 );
    Serial.printf("Scanning for server\n");

    doConnect = false;

    s_devicename = wifi.getDeviceName().c_str();
    s_pounce = false;
    s_heading = 0;
    s_latitude = 0;
    s_longitude = 0;

    pounced = false;

    mytime = millis();
    mynum = 0;

    Serial.println( "BLE setup finished" );
}

// Server

void BLEsupport::setupServer() 
{
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks( &srvCallbacks );
    NimBLEService* pService = pServer->createService( BLE_SERVER_UUID );
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic( BLE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE );
    pCharacteristic->setCallbacks(&chrCallbacksServer);
    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName( wifi.getDeviceName().c_str() );
    pAdvertising->addServiceUUID( pService->getUUID() );
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.println( "pAdvertising started" );
}

void BLEsupport::handleServerConnections() 
{
    if ( pServer == nullptr ) return;
    if ( pServer->getConnectedCount() == 0 ) return;
    NimBLEClient* pClient = pServer->getClient( 0 );
    if ( pClient == nullptr ) return;
    if ( ! pClient->isConnected() ) return;

    //Serial.println( "Client connected" );
}

bool BLEsupport::connectToServer() {
    NimBLEClient* pClient = nullptr;

    // Prevent self-connections
    uint8_t localMac[6];
    esp_efuse_mac_get_default(localMac);
    String advertisedDeviceAddress = advDevice->getAddress().toString().c_str();
    String localDeviceAddress = String(localMac[5], HEX) + ":" + String(localMac[4], HEX) + ":" + String(localMac[3], HEX) + ":" +
                                String(localMac[2], HEX) + ":" + String(localMac[1], HEX) + ":" + String(localMac[0], HEX);
    if (advertisedDeviceAddress == localDeviceAddress) {
        Serial.println("Not connecting to the server on the same device.");
        return false;
    }

    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount()) 
    {
        /**
         *  Note: NIMble examples say when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(advDevice, false)) {
                //Serial.printf("Reconnect failed\n");
                return false;
            }
            //Serial.printf("Reconnected client\n");
        } else {
            /**
             *  We don't already have a client that knows this device,
             *  check for a client that is disconnected that we can use.
             */
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient) {
        if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
            Serial.printf("Max clients reached - no more connections available\n");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        pClient->setClientCallbacks(&clientCallbacks, false);
        /**
         *  Note: NIMble says settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
         */
        pClient->setConnectionParams(12, 12, 0, 150);

        /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
        pClient->setConnectTimeout(5 * 1000);

        if (!pClient->connect( advDevice )) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            Serial.printf("Failed to connect, deleted client\n");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            Serial.printf("Failed to connect\n");
            return false;
        }
    }

    //Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    /** Now we can read/write/subscribe the characteristics of the services we are interested in */
    NimBLERemoteService*        pSvc = nullptr;
    NimBLERemoteCharacteristic* pChr = nullptr;
    NimBLERemoteDescriptor*     pDsc = nullptr;

    pSvc = pClient->getService( BLE_SERVER_UUID );
    if (pSvc) {
        pChr = pSvc->getCharacteristic( BLE_CHARACTERISTIC_UUID );
    }

    if (pChr) 
    {
        if (pChr->canWrite()) 
        {

            // Note: For now we send only the pounce value and heading, in the future 
            // we will connect, send a value, disconnect, repeat for devicename,
            // latitude, longitude. Do not try to pack all the data into one write,
            // NIMble 4.x sends maximum of 21 bytes
            // For the moment the client sends pounce:heading in one

            s_heading = compass.getHeading();

            String mydata;
            if ( pounced ) { mydata="true"; } else { mydata = "false"; }
            mydata += ":";
            mydata += String(s_heading, 0) ;

            /*
            Serial.print( "Client writing " );
            Serial.print( mydata );
            Serial.print( ", ");
            Serial.println( mydata.length() );
            */

            if ( pChr->writeValue( mydata.c_str() ) ) 
            {
                //Serial.printf("Wrote data to: %s\n", pChr->getUUID().toString().c_str());
            }
            
            pounced = false;
            pClient->disconnect();
            return true;
        }
    }

    return true;
}

bool BLEsupport::isCatNearby()
{
    if ( millis() - s_when < 3000 ) return true;
    return false;
}

bool BLEsupport::isPounced()
{
    if ( millis() - s_when < 3000 && s_pounce ) return true;
    return false;
}

void BLEsupport::sendPounce()
{
    pounced = true;
    s_when = millis();
}

bool BLEsupport::getPounce()
{
    return s_pounce;
}

float BLEsupport::getHeading()
{
    return s_heading;
}

float BLEsupport::getLatitude()
{
    return s_latitude;
}

float BLEsupport::getLongitude()
{
    return s_longitude;
}

String BLEsupport::getDevicename()
{
    return s_devicename;
}

int BLEsupport::getRSSI()
{
    return s_rssi;
}

void BLEsupport::loop() 
{
    if ( millis() - mytime > 5000 )
    {
        mytime = millis();
        handleServerConnections();    

        if ( doConnect || pounced ) 
        {
            doConnect = false;
            if ( connectToServer() ) 
            {
                // Serial.println("Send to server");
            } 
            else 
            {
                Serial.printf("Failed to connect, starting scan\n");
            }
            NimBLEDevice::getScan()->start(scanTimeMs, false, true);
        }

    }
}
