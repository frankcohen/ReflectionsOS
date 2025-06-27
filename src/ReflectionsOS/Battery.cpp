/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Battery.h"

Battery::Battery()
  : _voltageMv(0)
{}

void Battery::begin() 
{
  batck = millis();
  readVoltage_();
}

void Battery::loop() 
{
  if ( millis() - batck < 10000 ) return;
  batck = millis();

  readVoltage_();

  if (isBatteryLow()) {
    logger.info("Battery low (< " + String(batterylow) + "mV): signalling to sleep");
  }
}

void Battery::readVoltage_() {
  // analogReadMilliVolts returns mV; apply scale if your hardware needs it
  _voltageMv = analogReadMilliVolts(Battery_Sensor) * 5.4014;
}

uint16_t Battery::getVoltage() const {
  return _voltageMv;
}

bool Battery::isBatteryLow() {
  readVoltage_();
  return _voltageMv < batterysleep;
}

float Battery::getBatteryPercent() const {
  if (_voltageMv <= batterylow) return 0.0f;
  if (_voltageMv >= batteryfull) return 100.0f;
  return ( ( _voltageMv - static_cast<float>(batterylow) )
       / static_cast<float>(batteryfull - batterylow) )
       * 100.0f;
}

int Battery::getBatteryLevel() const {
  float v = static_cast<float>(_voltageMv);
  if (v < batterylow) return 1;
  if (v < batterymedium) return 2;
  return 3;
}

String Battery::getBatteryStats() const {
    // Read raw voltage and smooth to nearest 10 mV
    float rawMv = analogReadMilliVolts(Battery_Sensor) * 5.4014;
    int v = static_cast<int>(round(rawMv / 10.0f) * 10);  // smooth noise to 10 mV

    // Get and round percentage
    int pct = static_cast<int>(round(getBatteryPercent()));

    // Return voltage (smoothed) and percentage
    return "  " + String(v) + "mV, " + String(pct) + "%";
}

