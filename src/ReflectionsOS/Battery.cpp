/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include <Arduino.h>
#include <esp32-hal-adc.h>

#include "Battery.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>


Battery::Battery()
  : _voltageMv(0)
{}

void Battery::begin() 
{
  pinMode(Battery_Sensor, INPUT);

  analogSetPinAttenuation(Battery_Sensor, ADC_11db);

  // set up your periodic timer
  batck = millis();
  readVoltage_();
}

void Battery::readVoltage_() {
  _voltageMv = analogReadMilliVolts(Battery_Sensor) * 5.4014;
  
  //Serial.print( "Battery voltage: ");
  //Serial.println( _voltageMv );
}

uint16_t Battery::getVoltage() {
  return _voltageMv;
}

bool Battery::isBatteryLow() {
  readVoltage_();
  return _voltageMv < batterysleep;
}

float Battery::getBatteryPercent() 
{
  readVoltage_();
  if (_voltageMv <= batterylow) return 0.0f;
  if (_voltageMv >= batteryfull) return 100.0f;
  return ( ( _voltageMv - static_cast<float>(batterylow) )
       / static_cast<float>(batteryfull - batterylow) )
       * 100.0f;
}

// Returns 1-4 value depending on battery voltage
int Battery::getBatteryLevel() 
{
  readVoltage_();
  if ( _voltageMv < batterysleep ) return 1;
  if ( _voltageMv < batterylow ) return 2;
  if ( _voltageMv < batterymedium ) return 3;
  return 4;
}

String Battery::getBatteryStats() {
  // use your existing legacy readVoltage_()
  readVoltage_();  
  // _voltageMv is already scaled by 5.4014
  int v   = int((_voltageMv + 5) / 10) * 10;         // round to nearest 10 mV
  int pct = int(round(getBatteryPercent()));         // still calls readVoltage_()
  return "  " + String(v) + "mV, " + String(pct) + "%";
}

void Battery::loop() 
{
  if ( millis() - batck < 10000 ) return;
  batck = millis();

  if ( isBatteryLow()) {
    logger.info("Battery low (< " + String(batterylow) + "mV): signalling to sleep");
  }
}

