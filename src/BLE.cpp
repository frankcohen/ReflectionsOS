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

static String devicename;
static int nextState;
static uint32_t scanTime = 0; /** 0 = scan forever */
static NimBLEAdvertisedDevice* advDevice;
static String remoteDirection;
static String localDirection;

void BLE::setRemoteHeading( float myd )
{
  remoteHeading = myd;
}

float BLE::getRemoteHeading()
{
  return remoteHeading;
}

void BLE::setLocalHeading( float myd )
{
  localHeading = myd;
}

float BLE::getLocalHeading()
{
  return localHeading;
}

String BLE::getMessage()
{
  return message;
}

void BLE::setMessage( String _message )
{
  message = _message;
}

/** Callback to process the results of the last scan or restart it */
void scanEndedCB(NimBLEScanResults results)
{
  Serial.print( devicename );
  Serial.println(": scan ended");
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
   
  void onResult(BLEAdvertisedDevice* advertisedDevice) 
  {
    BLEUUID serviceUUID( SERVICE_UUID_CALLIOPE );

    // Does device contains the service we are looking for?
    if ( advertisedDevice -> haveServiceUUID() && advertisedDevice -> isAdvertisingService( serviceUUID ) )
    {
      Serial.print( devicename );
      Serial.print( ": BLE advertised device found: ");
      Serial.print( advertisedDevice->toString().c_str() );
      Serial.print( ", RSSI = " );
      Serial.println( advertisedDevice -> getRSSI() );

      BLEDevice::getScan()->stop();
      advDevice = advertisedDevice;
      nextState = 1;   // Next-up, connect to the server
    }
  }
};

class ServerCallbacks: public NimBLEServerCallbacks {

    void onConnect(NimBLEServer* pServer) 
    {
      Serial.print( devicename );
      Serial.println(": Server connected");
      NimBLEDevice::startAdvertising();
    };

    void onDisconnect(NimBLEServer* pServer) 
    {
      Serial.print( devicename );
      Serial.println(": Client disconnected, starting advertising");
      NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
      Serial.print( devicename );
      Serial.printf(": BLE MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };
};

class MyClientCallback: public BLEClientCallbacks {
  
  void onConnect(BLEClient* pclient) 
  {
    Serial.print( devicename );
    Serial.println(": onConnect");
  }

  void onDisconnect(BLEClient* pclient) 
  {
    nextState = 1;
    Serial.print( devicename );
    Serial.println(": onDisconnect");
  }

};

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks: public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) 
    {
      Serial.print( devicename );
      Serial.println(": Client callback reports connected");

      /** After connection we should change the parameters if we don't need fast response times.
        *  These settings are 150ms interval, 0 latency, 450ms timout.
        *  Timeout should be a multiple of the interval, minimum is 100ms.
        *  I find a multiple of 3-5 * the interval works best for quick response/reconnect.
        *  Min interval: 120 * 1.25ms = 150, Max interval: 120 * 1.25ms = 150, 0 latency, 60 * 10ms = 600ms timeout
        */
      pClient->updateConnParams(120,120,0,60);
    };

    void onDisconnect(NimBLEClient* pClient) 
    {
      Serial.print(pClient->getPeerAddress().toString().c_str());
      Serial.print( devicename );
      Serial.println(": Client callback reports disconnected");
      NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
    };

    /** Called when the peripheral requests a change to the connection parameters.
     *  Return true to accept and apply them or false to reject and keep
     *  the currently used parameters. Default will return true.
     */
    bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) {
        if(params->itvl_min < 24) { /** 1.25ms units */
            return false;
        } else if(params->itvl_max > 40) { /** 1.25ms units */
            return false;
        } else if(params->latency > 2) { /** Number of intervals allowed to skip */
            return false;
        } else if(params->supervision_timeout > 100) { /** 10ms units */
            return false;
        }

        return true;
    };

    /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
      Serial.print( devicename );
      Serial.println(": client Passkey request");
      /** return the passkey to send to the server */
      return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key){
      Serial.print( devicename );
      Serial.print(": passkey YES/NO number: ");
      Serial.println(pass_key);
    /** Return false if passkeys don't match. */
        return true;
    };

    /** Pairing process complete, we can check the results in ble_gap_conn_desc */
    void onAuthenticationComplete(ble_gap_conn_desc* desc){
        if(!desc->sec_state.encrypted) {
          Serial.print( devicename );
          Serial.println(": Encrypt connection failed - disconnecting");
          /** Find the client with the connection handle provided in desc */
          NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
          return;
        }
    };
};

/** Handler class for server characteristic actions */

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
      Serial.print( devicename );
      Serial.print(": onRead BLE server ");
      Serial.print(pCharacteristic->getUUID().toString().c_str());
      Serial.print(": onRead(), value: ");
      String remoteHeading = pCharacteristic->getValue().c_str();
      Serial.println( remoteHeading );
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
      Serial.print( devicename );
      Serial.print( ": onWrite ");
      Serial.print(pCharacteristic->getUUID().toString().c_str());
      Serial.print(": onWrite(), value: ");
      Serial.println(pCharacteristic->getValue().c_str());
    };
    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
      Serial.print( devicename );
      Serial.println(": Sending notification to clients");
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

      Serial.print( devicename );
      Serial.print( ": ");
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

/** Notification / Indication receiving handler callback */

void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
    std::string str = (isNotify == true) ? "Notification" : "Indication";
    str += " from ";
    /** NimBLEAddress and NimBLEUUID have std::string operators */
    str += std::string(pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress());
    str += ": Service = " + std::string(pRemoteCharacteristic->getRemoteService()->getUUID());
    str += ", Characteristic = " + std::string(pRemoteCharacteristic->getUUID());
    str += ", Value = " + std::string((char*)pData, length);
    Serial.println(str.c_str());
}

/** Create a single global instance of the callback class to be used by all clients */
static ClientCallbacks clientCB;


bool BLE::connectToServer() 
{
  if ( pClient == nullptr )
  {
    if( NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS ) 
    {
      Serial.print( devicename );
      Serial.println(": Max clients reached - no more connections available");
      return false;
    }

    pClient = NimBLEDevice::createClient();
    if ( ! pClient )
    {
      Serial.print( devicename );
      Serial.println(": Failed to create client");
      return false;
    }

    pClient->setClientCallbacks(&clientCB, false);

    Serial.print( devicename );
    Serial.println(": New client created");

    /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
      *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
      *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
      *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout

      pClient->setConnectionParams(40,200,0,200);

      The first 2 parameters are multiples of 0.625ms, they dictate how often a packet is sent to maintain the connection.
      The third lets the peripheral skip the amount of connection packets you specify, usually 0.

      The fourth is a multiple of 10ms that dictates how long to wait before considering the connection dropped if no packet is received, 
      I usually keep this around 100-200 (1-2 seconds).

      The last 2 parameters I would suggest not specifying, they are the scan parameters used when calling connect() as it will scan 
      to find the device you're connecting to, the default values work well.
      */

    pClient->setConnectionParams( 32,160,0,500 );
    /** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
    pClient->setConnectTimeout(15);
  }

  if ( ! pClient -> isConnected() ) 
  {
    Serial.print( devicename );
    Serial.println(": Not yet connected");

    if ( ! pClient -> connect( advDevice ) ) 
    {
      /** Created a client but failed to connect, don't need to keep it as it has no data */
      NimBLEDevice::deleteClient(pClient);
      Serial.print( devicename );
      Serial.println(": Failed to connect");
      return false;
    }
    else
    {
      Serial.print( devicename );
      Serial.println(": Client connecteed successfully to advertised service");
    }
  }
  else
  {
    Serial.print( devicename );
    Serial.println(": Client already connected");
    return false;
  }

  Serial.print( devicename );
  Serial.print(": Connected to: ");
  Serial.print(pClient->getPeerAddress().toString().c_str());
  //Serial.print(", RSSI: ");
  //Serial.println( pClient -> getRssi() );

  /** Now we can read/write/subscribe the charateristics of the services we are interested in */
  NimBLERemoteDescriptor* pDsc = nullptr;
  NimBLERemoteService* pSvc;
  NimBLERemoteCharacteristic* pChr;

  BLEUUID serviceUUID( SERVICE_UUID_CALLIOPE );
  BLEUUID charUUID( CHARACTERISTIC_UUID_HEADING );

  pSvc = pClient->getService( serviceUUID );
  if( pSvc )
  {
    pChr = pSvc->getCharacteristic( charUUID );

    if( pChr ) 
    {
      if( pChr -> canRead() )
      {
        String uume = pChr->getUUID().toString().c_str();
        String uuval = pChr->readValue().c_str();
        Serial.println( "" );
        Serial.print( devicename );
        Serial.print( ": " );
        Serial.print( uume );
        Serial.print( " reports value: ");
        Serial.print( uuval );
        Serial.print( ", local heading ");
        Serial.println( message );
      }

      /** registerForNotify() has been deprecated and replaced with subscribe() / unsubscribe().
        *  Subscribe parameter defaults are: notifications=true, notifyCallback=nullptr, response=false.
        *  Unsubscribe parameter defaults are: response=false.
        */
      if( pChr -> canNotify() )
      {
        if( !pChr -> subscribe( true, notifyCB ) )
        {
          Serial.print( devicename );
          Serial.print( ": subscribe failed" );

          pClient->disconnect();
          return false;
        }
      }        
      else if( pChr -> canIndicate() )
      {
        /** Send false as first argument to subscribe to indications instead of notifications */
        if( !pChr -> subscribe(false, notifyCB) ) 
        {
          Serial.print( devicename );
          Serial.print( ": subscribe failed on canIndicate" );

          pClient->disconnect();
          return false;
        }
      }
    }
  } 
  else
  {
    Serial.print( devicename );
    Serial.println(": Service not found.");
    return false;
  }

  Serial.print( devicename );
  Serial.println(": Connected" );
  return true;
}

/*
 *  Get direction value from the BLE server, print it to the Serial monitor
 */

bool BLE::getServerValue()
{
  if ( pClient == nullptr )
  {
    Serial.print( devicename );
    Serial.print( ": getServerValue pClient is null" );
  }
  else
  {
    BLEUUID serviceUUID( SERVICE_UUID_CALLIOPE );
    BLEUUID charUUID( CHARACTERISTIC_UUID_HEADING );

    NimBLERemoteService* pSvc2 = pClient -> getService( serviceUUID );
    if ( pSvc2 )
    {
      NimBLERemoteCharacteristic* pChr2 = pSvc2->getCharacteristic( charUUID );

      if( pChr2 ) 
      {
        if( pChr2 -> canRead() )
        {
          String meval = pChr2 -> readValue().c_str();
          remoteHeading = meval.toFloat();
          String uume = pChr2 -> getUUID().toString().c_str();

          Serial.println( "" );
          Serial.print( devicename );
          Serial.print( ": service " );
          Serial.print( uume );
          Serial.print(" reports value: " );
          Serial.print( remoteHeading );
          Serial.print( ", local heading ");
          Serial.println( message );
          return true;
        }
        else
        {
          Serial.print( devicename );
          Serial.print( ": GetServerValue ! canRead()" );
          return false;
        }
      }
      else
      {
        Serial.print( devicename );
        Serial.print( ": pChr2 null" );
        return false;
      }
    }
    else
    {
      Serial.print( devicename );
      Serial.print( ": pSvc2 null" );
        return false;
    }
  }
  return true;
}

static CharacteristicCallbacks chrCallbacks;

void BLE::begin()
{
  devname = host_name_me;
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  NimBLEDevice::init( devname );

  hottercolder = "";

  devicename = devname.c_str();

  advDevice = nullptr;
  pClient = nullptr;

  // Reset the RSSI running average
  for ( int j = 0; j < maxDistanceReadings - 1; j++ )
  {
    runningDistance[ j ] = 0;
  }

  localHeading = 361;
  remoteHeading  = 361;

  nextState = 0;
  msgCounter = 0;
  clientWaitTime = millis();
  serverWaitTime = millis();

  // Server set-up

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks( new ServerCallbacks() );

  NimBLEService* pService = pServer -> createService( SERVICE_UUID_CALLIOPE );

  pHeadingCharacteristic = pService -> createCharacteristic( CHARACTERISTIC_UUID_HEADING, NIMBLE_PROPERTY::READ );
  pHeadingCharacteristic -> setCallbacks( &chrCallbacks );

  pHeadingCharacteristic -> setValue( getMessage() );

  pService->start();

  Serial.print( devicename );
  Serial.println(": Server advertising starts");

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising -> addServiceUUID( pService -> getUUID() );
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
  NimBLEDevice::startAdvertising();

  // Client set-up

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan -> setAdvertisedDeviceCallbacks( new MyAdvertisedDeviceCallbacks() );
  pBLEScan -> setInterval(1349);
  pBLEScan -> setWindow(449);
  pBLEScan -> setActiveScan(true);
  pBLEScan -> start(5, false);

  Serial.print( devicename );
  Serial.println(": Begin finished");
}

bool BLE::matchHeading( float measured_angle ) 
{
  Serial.print( "matchheading: ");
  Serial.print( measured_angle );
  Serial.print( ", localHeading = " );
  Serial.println( localHeading );
 

  /* old way to evaluate heading
  //decoding heading angle according to datasheet
  if (measured_angle > 337.25 | measured_angle < 22.5)
  {
    //Serial.println("North");
    if ( localHeading == "S" ) return true;
    return false;
  }
  else {
    if (measured_angle > 292.5) {
      //Serial.println("North-West");
      if ( localHeading == "SE" ) return true;
      return false;
    }
    else if (measured_angle > 247.5) {
      //Serial.println("West");
      if ( localHeading == "E" ) return true;
      return false;
    }
    else if (measured_angle > 202.5) {
      //Serial.println("South-West");
      if ( localHeading == "NE" ) return true;
      return false;
    }
    else if (measured_angle > 157.5) {
      //Serial.println("South");
      if ( localHeading == "N" ) return true;
      return false;
    }
    else if (measured_angle > 112.5) {
      //Serial.println("South-East");
      if ( remoteHeading == "NW" ) return true;
      return false;
    }
    else if (measured_angle > 67.5) {
      //Serial.println("East");
      if ( localHeading == "W" ) return true;
      return false;
    }
    else {
      //Serial.println("North-East");
      if ( localHeading == "SW" ) return true;
      return false;
    }
  }
  */

  return false;
}

bool BLE::lookingTowardsEachOther( float avgDistance )
{
  if ( ( localHeading > 360 ) || ( remoteHeading > 360 ) )
  {
    return false;
  }

  // 1) Calculate angle

  float a = 200;
  float asquared = pow( a, 2 );
  float b = avgDistance;
  float bsquared = pow( b, 2 );
  float c = a;
  float csquared = pow( c, 2 );
  
  float tot = csquared - asquared - bsquared;
  float totfac = tot / ( -1 * ( 2 * a * b ) );
  float acostot = acos( totfac );
  float C = acostot * ( 180/PI );   // convert to degrees
  
  float Ctot = asquared - bsquared - csquared;
  float Ctotsmall = Ctot / ( -1 * ( 2 * b * c) );
  float acosCtot = acos( Ctotsmall );
  float A = acosCtot * ( 180 / PI );   // convert to degrees

  float B = 180 - C - A;

  /*
  Serial.print( "asquared = ");
  Serial.println( asquared );
  Serial.print( "bsquared = ");
  Serial.println( bsquared );
  Serial.print( "csquared = ");
  Serial.println( csquared );
  Serial.print( "tot = ");
  Serial.println( tot );
  Serial.print( "totfac = ");
  Serial.println( totfac );
  Serial.print( "acostot = ");
  Serial.println( acostot );
  Serial.print( "C = ");
  Serial.println( C );
  Serial.print( "Ctot = ");
  Serial.println( Ctot );
  Serial.print( "Ctotsmall = ");
  Serial.println( Ctotsmall );
  Serial.print( "acosCtot = ");
  Serial.println( acosCtot );
  Serial.print( "A = ");
  Serial.println( A );
  Serial.print( "B = ");
  Serial.println( B );
  */

  if ( ( ( localHeading - 40 ) < A ) && ( ( localHeading + 40 ) > A ) )
  {
    Serial.print( devicename );
    Serial.print( ": Devices are looking towards each other" );
    Serial.println( hottercolder );
    return true;
  }
  else
  {
    Serial.print( devicename );
    Serial.print( ": Devices are not looking towards each other" );
    Serial.println( hottercolder );
    return false;
  }
}

void BLE::areDevicesPointedToEachOther()
{
  float distanceTotal = 0;

  if ( pClient != nullptr )
  {
    if ( pClient -> isConnected() )
    {
      int d = abs( pClient -> getRssi() );

      if ( d != 0 )
      {
        for ( int j = 0; j < maxDistanceReadings - 1; j++ )
        {
          runningDistance[ j ] = runningDistance[ j+1 ];
          distanceTotal += runningDistance[ j ];
        }
        runningDistance[ maxDistanceReadings - 1 ] = d;
        distanceTotal += d;

        float averageDistance = distanceTotal / maxDistanceReadings;

        if ( ( ( oldAverageDistance - 2 ) < averageDistance ) && ( ( oldAverageDistance + 2 ) > averageDistance ) )
        {
          hottercolder = "";
        }
        else
        {
          if ( oldAverageDistance > averageDistance )
          {
            hottercolder = ", getting hotter";
          }
          else
          {
            hottercolder = ", getting colder";
          }
        }

        /*
        Serial.print( devicename );
        Serial.print( ": oldAverageDistance = ");
        Serial.print( oldAverageDistance );
        Serial.print( ", averageDistance = " );
        Serial.print( averageDistance );
        Serial.print( ", hottercolder = " );
        Serial.println( hottercolder );
        */

        oldAverageDistance = averageDistance;

        /*
        Serial.print( devicename );
        Serial.print( ": runningDistance " );
        for ( int k = 0; k < runningDistance; k++ )
        {
          Serial.print( runningDistance[ k ] );
          Serial.print( ", " );
        }
        Serial.println( averageDistance );
        */

        lookingTowardsEachOther( averageDistance );
      }
      else
      {
        Serial.print( devicename );
        Serial.print( ": Distance = 0" );
      }
    }
  }
  else
  {
    for ( int k = 0; k < maxDistanceReadings - 1; k++ )
    {
      runningDistance[ k ] = 0;
    }
  }
}

void BLE::loop()
{
  // Server
  if ((millis() - serverWaitTime) > 1000)
  {
    serverWaitTime = millis();

    if ( pHeadingCharacteristic != nullptr )
    {
      String myc = message;
      std::string mys = myc.c_str();
      pHeadingCharacteristic -> setValue( mys );
    }
    else
    {
      Serial.print( devicename );
      Serial.print( ": pHeadingCharacteristic is null" );
    }
  }

  // Client

  if ((millis() - clientWaitTime) > 1000)
  {
    clientWaitTime = millis();

    if ( nextState == 1 )
    {
      if ( connectToServer() )
      {
        nextState = 2;
      }
      else
      {
        nextState = 1;
      }
    }

    if ( nextState == 2 )
    {
      if ( getServerValue() )
      {
        nextState = 2;
      }
      else
      {
        nextState = 1;
      }
    }

    areDevicesPointedToEachOther();
  }
}