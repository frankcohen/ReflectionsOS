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

    void begin();  // Initialization method
    void loop();  // Method called repeatedly in the main loop
    void startExperience( int exper );
    int getCurrentState();
 
  private:
    std::vector<Experience*> experiences;
    Experience* currentExperience;

    enum State { SETUP, RUN, TEARDOWN, STOPPED, IDLE } currentState;

    void operateExperience();

    bool isBusy;  // Flag to indicate if a task is currently being executed

    enum ExperienceName
    {
      None,
      CatsPlay,
      PlayTime,
      EyesFollowFinger,
      ShowTime,
      ParallaxCat,
      Shaken,
      SwipeFinger,
      Chastise,
      UserCancels
    };

    enum ExperiencePhase 
    {
      Startup,
      Run,
      Teardown,
      Idle
    };
    
};

#endif   //INVEIGLE_H
