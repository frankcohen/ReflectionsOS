/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _EXPERIENCESERVICE_H
#define _EXPERIENCESERVICE_H

#include <Arduino.h>
#include <stdlib.h>  // For random number generation
#include <vector>

#include "Experience.h"
#include "ExperienceStats.h"
#include "Logger.h"
#include "Battery.h"
#include "WatchFaceMain.h"

extern Battery battery;  // External Battery class
extern LOGGER logger;
extern ExperienceStats experiencestats;
extern WatchFaceMain watchfacemain;

class ExperienceService
{
  public:
    ExperienceService();

    enum State { SETUP, RUN, TEARDOWN, STOPPED };

    void begin();  // Initialization method
    void loop();  // Method called repeatedly in the main loop
    void startExperience( int exper );
    int getExperNum();
    int getCurrentState();
    void setCurrentState( State state );
    int getExperience();
    bool active();
    void resetAfterTimer();
    unsigned long getAfterTimer();

    // Be certain to update experienceNameToString() when making changes to this structure
    enum ExperienceName
    {
      Awake,
      ShowTime,
      Sleep,
      Chastise,
      EyesFollowFinger,
      ParallaxCat,
      Hover,
      CatsPlay,
      MysticCat,
      Shaken,
      GettingSleepy,
      Pounce,
      ExperienceCount   // Used to iterate through the list,  add new elements above
    };

    String experienceNameToString( int experience);

  private:
    Experience* currentExperience;
    Experience* makeExp;
    State currentState;
    std::vector<Experience*> experiences;

    void operateExperience();

    int experienceIndex;  // used for testing

    int expernum;

    unsigned long catsplayTimer;
    unsigned long catsplayTimer2;
    unsigned long gestureTimer;
    unsigned long afterTimer;
    unsigned long noopTimer;
    bool noopFlag;

    unsigned long afterCatsPlay;
    unsigned long afterPounce;

    bool beingpounced;

    unsigned long temptimer;
};

#endif   // _EXPERIENCESERVICE_H
