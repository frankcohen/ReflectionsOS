/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_MysticCat_H
#define Experience_MysticCat_H

#include <Arduino.h>

#include "Experience.h"

#include "Logger.h"
#include "Video.h"
#include "TextMessageService.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TextMessageService textmessageservice;

static const char* const affirmative[][2] = 
{
  {"Even being mad",     "the answer, yes"},
  {"Oh, indeed, yes",    "silly you!"},
  {"Absolutely, yes",    "obviously"},
  {"A time for change.", "Yes dear yes"},
  {"Yes",                "without question."},
  {"Imagine reality",    "yes most yes"},
  {"My reality",         "oh, still yes!"},
  {"Tis certain", "what else?"},
  {"Indub-it-ably", "yes my dear!"},
  {"Affirmative", "surprisingly."},
};

static const char* const noncommital[][2] = 
{
  //12345678901234567    123456789012344567
  {"Proper order?",     "It's a mystery"},
  {"Haste makes waste", "Try again"},
  {"Not crazy",         "Don't see it yet"},
  {"Adventures require", "a first step"},
  {"Not all who wander", "are lost"},
  {"I prefer ", "the short-cut"},
  {"If you don't know", "neither do I"},
  {"I can't know", "everything"},
  {"Clear as a", "clouded night"},
  {"Yes... or no.", "Maybe both!"},
};

static const char* const negative[][2] = 
{
  {"Play fair?", "Not today"},
  {"I'm not all there", "so no, no no"},
  {"No time for joy", "nor play, no"},
  {"Right on time", "just not today"},
  {"Hidden in", "plain sight."},
  {"All are mad here", "dear."},
  {"It's late", "to be exact"},
  {"Wait, wait",    "you waited!"},
  {"Absolutely, Not", "No"},
};

#define mysticname F("Mystic ")

class Experience_MysticCat : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    String theMsg1;
    String theMsg2;
    bool vidflag;
    bool timeflag;
    
};

#endif // Experience_MysticCat
