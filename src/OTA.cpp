/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Thank you to @lipun12ka4 on Espressif Forums for sharing his OTA code,
 a problem, and solution:
 https://esp32.com/posting.php?f=13&mode=reply&t=30973&sid=89252e0558c707c8813146f40cccf9fc

*/

#include "OTA.h"

OTA::OTA(){}

void OTA::begin()
{ 
}
 
#define OTA_MAX_BYTES_PER_BATCH 4096
 
uint8_t buf[ OTA_MAX_BYTES_PER_BATCH + 1 ];
 
//static char *TAG = "OTA";
char const *TAG = "OTA";
 
static uint32_t total = 0;
static uint32_t pct = 0;
static uint32_t pct_send = 0;
static char message_data[10];
 
//static esp_err_t err;//Error variable to indicate when calling function return error.It is used in NVS function (WRITE,READ SERIAL number from NVS)

static esp_ota_handle_t update_handle = 0;  //should be used for subsequent esp_ota_write() and esp_ota_end() calls

//const esp_partition_t *update_partition = NULL; //Pointer to info for partition which should be updated next.

//NULL result indicates invalid OTA data partition, or that no eligible OTA app slot partition was found 

/*
  Uses ESP32 app_update to do local OTA, where Storage.replicate()
  already downloads the firmware and version number
  (.bin and file named OTA_VERSION_FILE_NAME).
  https://github.com/espressif/esp-idf/tree/master/components/app_update

  This does the installation using the OTA mechanism

  Upon successful update, stores the new version number to the version file
  and restarts the device.
*/

bool OTA::update()
{
  if ( WiFi.status() != 3 ) return false;

  //  Find the otaversion.txt file, return if it does not exist
  File versfile = SD.open( OTA_VERSION_FILE_NAME, FILE_READ );
  if( !versfile )
  {
    Serial.print( OTA_VERSION_FILE_NAME );
    Serial.println( F( " not found, skipping OTA update" ) );
    return false;
  }

  // Deserialize the version number
  char myNum[5];
  int i = 0;
  while ( versfile.available() && ( i < 5 ) )
  {
    myNum[ i ] = versfile.read();
    i++;
  }
  myNum[ i ] = 0;
  int myVer = atoi( myNum );

  Serial.print( F( "OTA version number from file is " ) );
  Serial.println( myVer );

  if ( myVer <= VERSION_NUMBER )
  {
    Serial.println( F( "No need to update." ) );
    return false;
  }

  // Update to the new firmware

  File ota_bin_file;
  int finished = 0;
  struct stat entry_stat;
  uint32_t file_size = 0;

  Serial.print( F( "OTA Bin File Path = " ) );
  Serial.println( OTA_BIN_FILE_NAME );

  ota_bin_file = SD.open( OTA_BIN_FILE_NAME, FILE_READ );

  if ( ! ota_bin_file ) 
  {
    Serial.print( F( "Failed to open OTA file: ") );
    Serial.println( OTA_BIN_FILE_NAME );
    return false;
  }
  
  file_size = (uint32_t) ota_bin_file.size();
  Serial.print( F( "OTA file size is " ) );
  Serial.println( file_size );
  
  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if ( configured != running )
  {
    Serial.print( F( "Configured OTA boot partition at offset" ) );
    Serial.print( configured->address, HEX );
    Serial.print( F( " but running from offset " ) );
    Serial.println( running->address, HEX );
    Serial.print( F( "This can happen from corrupted OTA boot data or preferred boot image data.") );
  }

  Serial.print( F( "Running partition type " ) );
  Serial.print( running->type );
  Serial.print( F( " sub->type " ) );
  Serial.print( running->subtype );
  Serial.print( F( " offset " ) );
  Serial.println( running->address );

  const esp_partition_t *update_partition = esp_ota_get_next_update_partition( NULL );
  if ( update_partition == NULL )
  {
    Serial.print( F( "OTA update_partition is NULL ") );
    return false;
  }

  Serial.print( F( "Writing to partition subtype " ) );
  Serial.print( update_partition->subtype );
  Serial.print( F( " at offset " ) );
  Serial.println( update_partition->address );

  esp_err_t err = esp_ota_begin( update_partition, file_size, &update_handle );

  Serial.print( F( "esp_begin result = " ) );
  Serial.println( err );

  Serial.print( F( "Update handle is " ) );
  Serial.println( update_handle );

  if ( err != ESP_OK )
  {
    Serial.print( F( "esp_ota_begin failed, error " ) );
    Serial.println( err );
    return false;        
  }

  Serial.println( F( "esp_ota_begin succeeded" ) );

  while( !finished )
  {
    unsigned int retn = ota_bin_file.read( buf, OTA_MAX_BYTES_PER_BATCH );
    Serial.print( F( "First Byte of the Read Data Chunk is " ) );
    Serial.println( buf[0] );

    if ( retn != OTA_MAX_BYTES_PER_BATCH )
    {
      Serial.print( F( "Finished reading, last chunk size " ) );
      Serial.println( retn );

      err = esp_ota_write(update_handle, buf, retn);

      if (err == ESP_ERR_INVALID_ARG) 
      {
        Serial.print( F( "error: esp_ota_write failed! err = " ) );
        Serial.println( err );
        return false;
      } 
      else if (err == ESP_ERR_OTA_VALIDATE_FAILED) 
      {
        Serial.println( F( "error: First byte of image contains invalid app image magic byte" ) );
        return false;
      } 
      else if (err == ESP_ERR_FLASH_OP_FAIL)
      {
        Serial.println( F( "error: Flash write IO Operaion failed" ) );
        return false;
      } 
      else if ( err == ESP_ERR_FLASH_OP_TIMEOUT )
      {
        Serial.println( F( "error: Flash write failed due to TimeOut") );
        return false;
      }
      else if (err == ESP_ERR_OTA_SELECT_INFO_INVALID )
      {
        Serial.println( F( "error: OTA data partition has invalid contents" ) );
        return false;
      }
      else if (err == ESP_OK) 
      {
        Serial.print( F( "Wrote " ) );
        Serial.print( retn );
        Serial.println( F( " bytes to OTA Partition" ) );
      }

      Serial.print( F( "Ota result = " ) );
      Serial.println( err );

      total += retn;
      pct = total * 100 / file_size;
      itoa( pct, message_data, 10 );

      Serial.print( F( "Progress " ) );
      Serial.print( message_data) ;
      Serial.print( F( "/ttotal bytes read from the OTA BIN rile is " ) );
      Serial.println( total );

      delay( 2000 );

      Serial.print( F( "Update handle is " ) );
      Serial.println( update_handle );
      err = esp_ota_end( update_handle );

      if (err != ESP_OK) 
      {
        if ( err == ESP_ERR_OTA_VALIDATE_FAILED ) 
        {
          Serial.println( F( "Image validation failed, image corrupt" ) );
        }
        else
        {
          Serial.print( F( "esp_ota_end failed " ) );
          Serial.println( esp_err_to_name( err ) );
        }
        return false;
      }

      Serial.println( F( "OTA update ended" ) );

      err = esp_ota_set_boot_partition( update_partition );
      if ( err != ESP_OK )
      {
        Serial.print( F( "esp_ota_set_boot_partition failed, " ) );
        Serial.println( esp_err_to_name( err ) );
      }

      Serial.println( F( "Prepare to restart" ) );
      delay(5000);
      finished = 1;
      break;
    }
    else
    {
      err = esp_ota_write(update_handle, buf, retn);

      if ( err == ESP_ERR_INVALID_ARG )
      {
        Serial.print( F( "esp_ota_write failed. err= " ) );
        Serial.println( err );
        return false;
      }
      else if ( err == ESP_ERR_OTA_VALIDATE_FAILED )
      {
        Serial.print( F( "error: First byte of image contains invalid app image magic byte" ) );
        return false;
      }
      else if ( err == ESP_ERR_FLASH_OP_FAIL )
      {
        Serial.print( F( "error: Flash write IO Operaion failed" ) );
        return false;
      } 
      else if ( err == ESP_ERR_FLASH_OP_TIMEOUT )
      {
        Serial.print( F( "error: Flash write failed due to TimeOut" ) );
        return false;
      }
      else if ( err == ESP_ERR_OTA_SELECT_INFO_INVALID )
      {
        Serial.print( F( "error: OTA data partition has invalid contents" ) );
        return false;
      } 
      else if ( err == ESP_OK )
      {
        Serial.print( F( "Wrote " ) );
        Serial.print( retn );
        Serial.println( F( " bytes to OTA Partition" ) );
      }

      total += OTA_MAX_BYTES_PER_BATCH;

      pct = total * 100 / file_size;

      if( pct != pct_send )
      {
        pct_send = pct;
        itoa(pct, message_data, 10);
        Serial.print( F( "Progress " ) );
        Serial.println( message_data );
      }
    }
  }

  // Save the new version number to the otaversion file

  Serial.println( F( "Save the new version number to the otaversion file" ) );

  if( SD.remove( OTA_VERSION_FILE_NAME ) )
  {
    Serial.print("Deleted ");
    Serial.println( OTA_VERSION_FILE_NAME );

    File versfile = SD.open( OTA_VERSION_FILE_NAME, FILE_WRITE );
    if( !versfile )
    {
      Serial.println( F( "Failed to open OTA VERSION file for writing" ) );
      return false;
    }

    char numberArray[20];
    itoa( VERSION_NUMBER, numberArray, 10);
    versfile.print( numberArray );

    Serial.println( F( "Done" ) );
  } 
  else
  {
    Serial.print( F( "Failed to delete " ) );
    Serial.println( OTA_VERSION_FILE_NAME );
    return false;
  }

  Serial.println( F( "Update finished" ) );
  Serial.println( F( "Restarting the host" ) );
  ESP.restart();
  return true;
}
