# Arduino and BLE on ESP32 as server and client combined, using NimBLE

Frank Cohen, [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS), July 21, 2023

[NimBLE](https://mynewt.apache.org/latest/network/) is an open source Bluetooth Low Energy (BLE) stack for microcontrollers. [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) is a fork for Arduino compilation and for use with ESP32 and nRF5x. It is fully compliant with Bluetooth 5 specifications with support for Bluetooth Mesh.

NimBLE comes from the [Apache Mynewt](https://mynewt.apache.org/) real time operating system project. Mynewt is similar to other efforts like [Zephyr](https://zephyrproject.org/). It continues to be maintained in 2023 and issues posed to the Github repository go answered.

I recommend using NimBLE over [Espressif BLE](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/index.html). NimBLE is free, open-source, and supported. NimBLE uses less ESP32 memory. And NimBLE is API comnpatible with your source code, although you will be typing 'Nim' a lot, hehe.

## BLE is complicated, and so is NimBLE

It takes me 831 lines of C/C++ code to make my device act as a client and server using NimBLE. There is plenty of room in the world for a NimBLE abstraction layer. I have not found one yet. I would make a layer to set-up a service in 3 lines of source: name the device, identify the service IDs, and register a value to be shared to other BLE devices. With your encouragement I would write such a layer and contribute it to NimBLE.

The general object flow is to create a NimBLE object, attach characteristics to identify what the BLE service provides or consumes, and handle the communication through a set of callbacks.

As a BLE server, set-up these services

* Create the server
* Set-up callbacks
  * Connection callback
  * Values changed callback
* Create the service and characteristics
* Advertise the device capabilities
* Set the values you need the service to provide

As a BLE cliient, set-up these services

* Start scanning for servers advertising the service and characteristics you need
* When you find the service and characteristics, create the client, stop scanning
* Set-up callbacks
* Connect to a server
  * Create or reuse a client
  * Connect to the server
  * Read/write/subscribe the charateristics of the services we are interested in

## Limitations

### Unique host names and logging
Starting up a BLE device NimBLE needs a device name. You could name all the devices the same and that led me to a fair amount of confusion when debugging. I added the last 2 digits of the device MAC address. For example,

```
devname = "CALLIOPE-";
std::string mac = WiFi.macAddress().c_str();
devname.append( mac.substr( 15, 2 ) );
NimBLEDevice::init( devname );
```

and when logging I use,

```
Serial.print( devicename );
Serial.println(": Server advertising starts");
```

The logs then appear as,

```
CALLIOPE-8A: Server advertising starts
```

### Connection tuning

NimBLE supports connection parameters from the BLE standard. I set initial connection parameters to 15ms interval, 0 latency, and 120ms timout. These settings appear to be safe for 3 clients to connect reliably. Parameters impact BLE power consumption. And, they can go faster if you have less connections. Timeout should be a multiple of the interval, minimum is 100ms. Minimum interval: 12 * 1.25ms = 15, max interval: 51 * 10ms = 510ms timeout. Unfortunately the directions on how to tune the parameter values is a mess of contradictory instructions. I didn't find the final word on the parameter values after a week of hunting.

`
pClient->setConnectionParams(40,200,0,200);
`

I found instructions like this: The first 2 parameters are multiples of 0.625ms, they dictate how often a packet is sent to maintain the connection. The third lets the peripheral skip the amount of connection packets you specify, usually 0. The fourth is a multiple of 10ms that dictates how long to wait before considering the connection dropped if no packet is received, I usually keep this around 100-200 (1-2 seconds). The last 2 parameters I would suggest not specifying, they are the scan parameters used when calling connect() as it will scan to find the device you are connecting to, the default values work well.

`
pClient->setConnectionParams( 32,160,0,500 );
`

How long we are willing to wait for the connection to complete (seconds), default is 30.

```
pClient->setConnectTimeout(15);
```

### Client Connection Tuning

NimBLE supports connection tuning on the client side. I found multiple contradictory instructions on the best values. I set the initial connection parameters to 15ms interval, 0 latency, 120ms timout. These settings appear to be safe for 3 clients to connect reliably, can go faster if you have less connections. Timeout should be a multiple of the interval, minimum is 100ms. For example, the minimum interval is 12 * 1.25ms = 15, Max interval is 12 * 1.25ms = 15, 0 latency, and 51 * 10ms = 510ms timeout.

```
pClient->setConnectionParams(40,200,0,200);
```

The first 2 parameters are multiples of 0.625ms, they dictate how often a packet is sent to maintain the connection. The third lets the peripheral skip the amount of connection packets you specify, often 0.

The fourth is a multiple of 10ms that dictates how long to wait before considering the connection dropped if no packet is received, I keep this around 100-200 (1-2 seconds).

The last 2 parameters various documentation suggests not specifying, The scan parameters used when calling connect() as it will scan to find the device you're connecting to, the default values work well for my application.

### Error messages
Error handling is difficult, since NimBLE reports errors by messages to the Serial Monitor (stderr). [NimBLE host error messages](https://mynewt.apache.org/latest/network/ble_hs/ble_hs_return_codes.html)

### RSSI distancc readings

[Bluetooth RSSI](https://www.bluetooth.com/blog/proximity-and-rssi/) (Received Signal Strength Indicator) measures the quality level of a Bluetooth signal. It's handy to use it as a measure of the distance between devices. RSSI is measured in negative numbers (for example, -5). RSSI value -5 is very close and anything below -90 means the device is far away and most likely not reachable.

I found NimBLE RSSI readings on ESP32-S3 devices to be widely inaccurate. Taking an average of the past 3 RSSI values improved the data quality.

```
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
...
```

### String, string, std::string, ugh

Many NimBLE APIs and return values use std::string instead of String. It's just annoying, and leads to code like this:

```
String myc = message;
std::string mys = myc.c_str();
pHeadingCharacteristic -> setValue( mys );
```

## Things I have Not Worked Out

### Static Global Variables and callbacks

My code uses static global variables. NimBLE uses callbacks a lot. I am not expert enough in C/C++ to avoid static variables. Combing object programming with static globals is a very bad practice. Objects need to control their data. Statics make the data available and set=abe from any other code outside of the object.

### Additional issues

I need to balance the active scanning to improve battery life.

I need to make the code work among multiple devices, past the 2 devices it currently supports.

ESP32 BLE controller connects to up to 9 devices, 3 is the default max in IDF. You can change it using NIMBLE_MAX_CONNECTIONS. NimBLE is memory efficient, compared to the default BLE stack. I suspect setting the max connections higher also reserves more ram for the controller.

## Comments and feedback encouraged

My work with NimBLE is for the [Reflections](https://github.com/frankcohen/ReflectionsOS) project. Reflections is an open-source mobile entertainment experience development stack. I encourage you to send me your comments and feedback.

Many times I imagine making a mobile application connected to the Internet. For example, a wrist watch that shows videos of my children growing-up. I love the Arduino community and yet when I go to put something like this together I find many tutorials on the parts (like a display, Bluetooth, SD card, sound, and compass) there's very little I found that shows them integrated together. I am making my watch project and will be posting the code, datasheets, and design in the Reflections repository.

I love working with a group of creative makers. Please feel free to let me know your feedback and ideas. Thanks, in advance!

## Source Code

Below is the source code I wrote. It is part of the code I wrote for [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS) and does not compile by itself - I include it as an example to write your own code. I am distributing it under a GPL v3 license - free software, when you change the code you need to publish your code to the world. I use the code in my Reflection's open source project at https://github.com/frankcohen/ReflectionsOS in the BLE.h and BLE.cpp code files.

Enjoy,

-Frank

BLE.h code:

```
#ifndef _BLE_
#define _BLE_

#include "Arduino.h"
#include <WiFi.h>

#include "config.h"
#include "secrets.h"

#include <NimBLEDevice.h>

#define maxDistanceReadings 3

class BLE
{
  public:
    BLE();
    void begin();
    void loop();
    String getMessage();
    bool connectToServer();
    bool getServerValue();
    void setRemoteHeading( float myh );
    float getRemoteHeading();
    void setLocalHeading( float myd );
    float getLocalHeading();

  private:
    std::string devname;
    int msgCounter;
    unsigned long serverWaitTime;
    unsigned long clientWaitTime;
    String message;
    NimBLECharacteristic* pHeadingCharacteristic;
    NimBLEServer* pServer;
    NimBLEClient* pClient;
    int runningDistance [ maxDistanceReadings ];
    float localHeading;
    float remoteHeading;
};

#endif // _BLE_
```

BLE.cpp code:
```
/*
 Reflections, mobile entertainment experiences

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
  devname = "CALLIOPE-";
  std::string mac = WiFi.macAddress().c_str();
  devname.append( mac.substr( 15, 2 ) );
  NimBLEDevice::init( devname );

  hottercolder = "";

  devicename = devname.c_str();

  advDevice = nullptr;
  pClient = nullptr;

  // Rest the RSSI running average
  for ( int j = 0; j < maxDistanceReadings - 1; j++ )
  {
    runningDistance[ j ] = 0;
  }

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

  }
}
```


