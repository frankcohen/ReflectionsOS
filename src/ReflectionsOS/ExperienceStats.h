/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef EXPERIENCESTATS_H
#define EXPERIENCESTATS_H

#include <Arduino.h>
#include <vector>
#include <Preferences.h>

/**
 * @brief Tracks how often each named Experience is started and reports stats to Serial.
 */
class ExperienceStats {
public:
    /**
     * @param reportIntervalMs How often to send stats (ms), default 60000 (1 min)
     */
    ExperienceStats(unsigned long reportIntervalMs = 60000UL);

    /**
     * Initialize Serial and timer. Call from setup().
     */
    void begin(unsigned long currentMs = millis());

    /**
     * Record a start of the experience by name.
     */
    void record(const String& name);

    /**
     * Must be called frequently (e.g., each loop). Will print stats if interval elapsed.
     */
    void update(unsigned long currentMs = millis());

    bool isTerri();
    bool isFrank();

    unsigned long totalCalls_;

private:
    void printStats() const;

    unsigned long intervalMs_;
    unsigned long lastReportMs_;

    std::vector<String> names_;
    std::vector<unsigned long> counts_;

    Preferences prefs_;     // Retains stats between ESP32 deep sleep modes

    unsigned long lastTerriCall;
    unsigned long lastFrankCall;
};

#endif // EXPERIENCESTATS_H
