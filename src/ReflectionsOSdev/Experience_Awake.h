/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_Awake_H
#define Experience_Awake_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"

#define awakename F("Awake ")

class Experience_Awake : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    
};

#endif // Experience_Awake_H
