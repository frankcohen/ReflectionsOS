/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef Experience_ShowTime_H
#define Experience_ShowTime_H

#include <Arduino.h>

#include "Experience.h"
#include "TextMessageService.h"

#include "Logger.h"
#include "Video.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern TextMessageService textmessageservice;

// Digital clock fun messages

static const char* const timefunmessages[][2] = 
{
  { "It's early",    "to be exact" },
  { "It's late",     "to be exact"},
  { "Wait, wait",    "you waited!"},
  { "Peekaboo",      "i see you"},
  { "Why?",          "why not?"},
  { "When?",         "and where?"},
  { "Little time",   "to be exact"},
  { "Will it end?",  "and when?"},
  { "Cats forever",  "meow"},
  { "Hug please",    "forever"},
  { "I'm late",      "important date"},
  { "No panic",      "no worries"},
  { "Grin",          "like a cat"},
  { "Curiouser",     "and curiouser"},
  { "A dream",       "within a dream"},
  { "I've gone",     "entirely mad"},
  { "All mad",       "all of us"},
  { "Simply mad",    "as a hatter"},
  { "Twiddledee",    "and Twiddledum"},
  { "I vanish",      "like a ghost"},
  { "Very, very",    "mysterious"},
  { "Truly, very",   "wonderland"},
  { "I appear",      "and disappear"},
  { "Slithy toves",  "Brillig?"},
  { "Did gyre?",     "Borogroves?"},
  { "Play time?",    "Croquet"},
  { "Follow",        "White rabbit"},
  { "See me?",       "Or not"},
  { "A rabbit?",     "White even?"},
  { "I am mad",      "So are you"},
  { "Find me",       "if you can"},
  { "A place",       "like no other"},
  { "Do you?",       "Believe?"},
  { "Look closely",  "See nothing"},
  { "Smiling",       "Ear to ear"},
  { "Invisible!",    "Yet here"},
};

#define showtimename F("Show Time ")

class Experience_ShowTime : public Experience {
  public:
    void setup() override;
    void run() override;
    void teardown() override;
    void init() override;

  private:
    unsigned long timer;
    bool tearflag;
    bool timeflag;
    bool vidflag;
    
};

#endif // Experience_ShowTime_H
