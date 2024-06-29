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
    void overrideExperience(Experience* newExperience);

  private:
    std::vector<Experience*> experiences;
    Experience* currentExperience;
    void clearCurrentExperience();

    void operateExperience();

    enum State { SETUP, RUN, TEARDOWN, STOPPED } currentState;

    unsigned long previousMillis;  // Variable to store the last time a task was executed
    unsigned long idleStartMillis;  // Variable to store the start time of the idle period
    unsigned long noGestureStartTime;  // Variable to store the start time when no gestures were detected

    const long interval = 1000;    // Interval at which to perform tasks (in milliseconds)
    const long idleTimeout = 5000; // 5 seconds of idle time to trigger sleepTime
    const long noGestureTimeout = 120000; // 2 minutes (120000 milliseconds) for no gestures detection

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
