/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef EXPERIENCESERVICE_H
#define EXPERIENCESERVICE_H

#include <Arduino.h>
#include <stdlib.h>  // For random number generation
#include <vector>

#include "Experience.h"

#include "Logger.h"
#include "TOF.h"
#include "AccelSensor.h"
#include "Battery.h"
#include "BLEsupport.h"
#include "TextMessageService.h"

extern Battery battery;  // External Battery class
extern LOGGER logger;
extern AccelSensor accel;
extern TOF tof;
extern BLEsupport blesupport;
extern Video video;
extern TextMessageService textmessageservice;

/*
ChatGPT says RSSI values:
-30 dBm to -50 dBm: Very strong signal; the devices are very close (within a few centimeters to about 1 meter).
-50 dBm to -70 dBm: Strong signal; the devices are likely within 1 to 5 meters.
-70 dBm to -90 dBm: Moderate signal; the devices might be within 5 to 10 meters.
-90 dBm and below: Weak signal; the devices are likely farther apart or there are significant obstacles/interference.
*/

class ExperienceService 
{

  public:
    ExperienceService();  // Constructor

    enum State { SETUP, RUN, TEARDOWN, STOPPED };
    void begin();  // Initialization method
    void loop();  // Method called repeatedly in the main loop
    void startExperience( int exper );
    int getCurrentState();
    void setCurrentState( State state );
    bool active();
    void resetAfterTimer();

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

    unsigned long catsplayTimer;
    unsigned long catsplayTimer2;
    unsigned long gestureTimer;
    unsigned long afterTimer;
    unsigned long noopTimer;
    bool noopFlag;

    unsigned long afterCatsPlay;
    unsigned long afterPounce;

    bool beingpounced;
    unsigned long pnctimer;
};

#endif   // EXPERIENCESERVICE_H
