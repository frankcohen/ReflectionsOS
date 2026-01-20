/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "ExperienceStats.h"

ExperienceStats::ExperienceStats(unsigned long reportIntervalMs)
  : intervalMs_(reportIntervalMs), totalCalls_(0), lastReportMs_(0), lastTerriCall(0), lastFrankCall(0) {}

void ExperienceStats::begin(unsigned long currentMs) 
{
  lastReportMs_ = currentMs;

  // 1) Open our NVS namespace
  prefs_.begin("expStats", false);

  // 2) Recover totalCalls
  totalCalls_ = prefs_.getULong("totalCalls", 0);

  // 3) Recover lastTerriCall and lastFrankCall
  lastTerriCall = prefs_.getULong("lastTerriCall", 0);
  lastFrankCall = prefs_.getULong("lastFrankCall", 0);

  // 4) Recover how many distinct names we stored
  uint16_t numNames = prefs_.getUShort("numNames", 0);
  for (uint16_t i = 0; i < numNames; i++) {
    char keyName[16];
    sprintf(keyName, "name%u", i);
    String name = prefs_.getString(keyName, "");
    
    sprintf(keyName, "cnt%u", i);
    unsigned long cnt = prefs_.getULong(keyName, 0);

    names_.push_back(name);
    counts_.push_back(cnt);
  }

  terrifrank = 0;
}

void ExperienceStats::record(const String& name) 
{
  totalCalls_++;

  // find or append
  for (size_t i = 0; i < names_.size(); ++i) {
    if (names_[i] == name) {
      counts_[i]++;
      
      // persist this single counter
      char key[16];
      sprintf(key, "cnt%u", (unsigned)i);
      prefs_.putULong(key, counts_[i]);
      
      // persist totalCalls
      prefs_.putULong("totalCalls", totalCalls_);
      return;
    }
  }

  // new name: append vectors and persist for ESP32 deep sleep modes
  
  size_t idx = names_.size();
  names_.push_back(name);
  counts_.push_back(1);

  // save the string and its count
  char key[16];
  sprintf(key, "name%u", (unsigned)idx);
  prefs_.putString(key, name);

  sprintf(key, "cnt%u", (unsigned)idx);
  prefs_.putULong(key, 1);

  // update number of names
  prefs_.putUShort("numNames", (uint16_t)names_.size());
  
  // persist totalCalls
  prefs_.putULong("totalCalls", totalCalls_);
}

void ExperienceStats::update(unsigned long currentMs) {
    if (currentMs - lastReportMs_ >= intervalMs_) {
        printStats();
        lastReportMs_ = currentMs;
    }
}

bool ExperienceStats::isTerri()
{
  // Only return true when totalCalls is a multiple of 30, and it hasn't been triggered already
  if ((totalCalls_ % 30) == 0 && totalCalls_ != lastTerriCall) 
  {
    lastTerriCall = totalCalls_;  // Update the lastTerriCall to avoid multiple triggers
    prefs_.putULong("lastTerriCall", lastTerriCall);  // Persist lastTerriCall
    return true;
  }
  return false;
}

bool ExperienceStats::isFrank()
{
  // Only return true when totalCalls is a multiple of 50, and it hasn't been triggered already
  if ((totalCalls_ % 50) == 0 && totalCalls_ != lastFrankCall) {
    lastFrankCall = totalCalls_;  // Update the lastFrankCall to avoid multiple triggers
    prefs_.putULong("lastFrankCall", lastFrankCall);  // Persist lastFrankCall
    return true;
  }
  return false;
}

void ExperienceStats::printStats() const {
    Serial.println(F("=== Experience Statistics ==="));
    for (size_t i = 0; i < names_.size(); ++i) {
        float pct = totalCalls_ > 0 ? (counts_[i] * 100.0f / totalCalls_) : 0.0f;
        Serial.print(names_[i]);
        Serial.print(F(": "));
        Serial.print(counts_[i]);
        Serial.print(F(" ("));
        Serial.print(pct, 1);
        Serial.println(F("%)"));
    }
    Serial.print(F("Total calls: "));
    Serial.println(totalCalls_);
    
    Serial.print( "lastFrank: " );
    Serial.print( lastFrankCall );
    Serial.print( " lastTerri: " );
    Serial.println( lastTerriCall );
    
    Serial.println(F("============================="));
}
