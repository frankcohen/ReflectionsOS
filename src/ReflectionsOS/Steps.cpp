/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Step counter using accelerometer

*/

#include "Steps.h"

Steps::Steps(){}

void Steps::begin()
{ 
  loadFromNVS();    // Load saved step count
  lastStepTime = millis();
}

bool Steps::saveToNVS() {
    nvs_handle handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        Serial.printf("[Steps] NVS open failed: %d\n", err);
        return false;
    }
    err = nvs_set_u32(handle, "stepCount", stepCount);
    if (err != ESP_OK) {
        Serial.printf("[Steps] NVS set failed: %d\n", err);
        nvs_close(handle);
        return false;
    }
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        Serial.printf("[Steps] NVS commit failed: %d\n", err);
        nvs_close(handle);
        return false;
    }
    nvs_close(handle);
    return true;
}

bool Steps::loadFromNVS() {
    nvs_handle handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        Serial.printf("[Steps] NVS open failed: %d\n", err);
        stepCount = 0;
        return false;
    }
    uint32_t stored = 0;
    err = nvs_get_u32(handle, "stepCount", &stored);
    if (err == ESP_OK) {
        stepCount = stored;
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        stepCount = 0;
    } else {
        Serial.printf("[Steps] NVS read failed: %d\n", err);
        stepCount = 0;
    }
    nvs_close(handle);
    return true;
}

void Steps::loop()
{
  uint32_t currentTime = millis();

  if ( currentTime - lastStepTime > 500 ) 
  {
    lastStepTime = currentTime;

    float x = accel.getXreading();
    float y = accel.getYreading();
    float z = accel.getZreading();

    // Calculate magnitude of acceleration
    float magnitude = sqrt(x * x + y * y + z * z);

    /*
    Serial.print( F("Steps ") );
    Serial.print( stepCount );
    Serial.print( F(", mag ") );
    Serial.println( magnitude );
    */
        
    // Detect steps based on threshold and timing
    if ( magnitude >= accelThreshold ) 
    {
      stepCount++;
      saveToNVS();
    }
  }
}

int Steps::howManySteps() 
{
  return stepCount;
}

void Steps::resetStepCount() 
{
  saveToNVS();
  stepCount = 0;
}
