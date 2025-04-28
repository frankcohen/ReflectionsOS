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
  : intervalMs_(reportIntervalMs), totalCalls_(0), lastReportMs_(0) {}

void ExperienceStats::begin(unsigned long currentMs) {
    Serial.begin(115200);
    while (!Serial);
    lastReportMs_ = currentMs;
}

void ExperienceStats::record(const String& name) {
    totalCalls_++;
    // Search existing names
    for (size_t i = 0; i < names_.size(); ++i) {
        if (names_[i] == name) {
            counts_[i]++;
            return;
        }
    }
    // New experience name
    names_.push_back(name);
    counts_.push_back(1);
}

void ExperienceStats::update(unsigned long currentMs) {
    if (currentMs - lastReportMs_ >= intervalMs_) {
        printStats();
        lastReportMs_ = currentMs;
    }
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
    Serial.println(F("============================="));
}
