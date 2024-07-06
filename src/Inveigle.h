#ifndef INVEIGLE_H
#define INVEIGLE_H

#include <Arduino.h>
#include <stdlib.h>  // For random number generation
#include <vector>

#include "Experience.h"

#include "Logger.h"
#include "TOF.h"
#include "Accelerometer.h"
#include "Battery.h"

extern Battery battery;  // External Battery class
extern LOGGER logger;
extern Battery battery;
//extern Hardware hardware;
extern Accelerometer accel;
extern TOF tof;

class Inveigle 
{

  public:
    Inveigle();  // Constructor

    enum State { SETUP, RUN, TEARDOWN, STOPPED };
    void begin();  // Initialization method
    void loop();  // Method called repeatedly in the main loop
    void startExperience( int exper );
    int getCurrentState();
    void setCurrentState( State state );

    enum ExperienceName
    {
      Awake,
      ShowTime,
      Sleep,
      CatsPlay,
      MysticCat,
      EyesFollowFinger,
      ParallaxCat,
      Shaken,
      SwipeFinger,
      Chastise
    };

  private:
    Experience* currentExperience;
    State currentState;
    std::vector<Experience*> experiences;

    void operateExperience();

    bool sleepStarting;

    unsigned long msitime;
    unsigned long msiwhen;
    
    unsigned long toftimer;
};

#endif   //INVEIGLE_H
