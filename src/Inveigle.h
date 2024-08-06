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
#include "BLE.h"

extern Battery battery;  // External Battery class
extern LOGGER logger;
extern Battery battery;
//extern Hardware hardware;
extern Accelerometer accel;
extern TOF tof;

/*
ChatGPT says RSSI values:
-30 dBm to -50 dBm: Very strong signal; the devices are very close (within a few centimeters to about 1 meter).
-50 dBm to -70 dBm: Strong signal; the devices are likely within 1 to 5 meters.
-70 dBm to -90 dBm: Moderate signal; the devices might be within 5 to 10 meters.
-90 dBm and below: Weak signal; the devices are likely farther apart or there are significant obstacles/interference.
*/

#define catsplayClose -30
#define catsplayCloser -50

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
      SetTime,
      Chastise,
      EyesFollowFinger,
      ParallaxCat,
      SwipeFinger,
      CatsPlay,
      MysticCat,
      Shaken,
      GettingSleepy,
      Pounce,
      ExperienceCount   // Used to iterate through the list,  add new elements above
    };

  private:
    Experience* currentExperience;
    Experience* makeExp;
    State currentState;
    std::vector<Experience*> experiences;

    void operateExperience();

    int experienceIndex;  // used for testing
    unsigned long klezcount;

    unsigned long catsplayTimer;    

    bool sleepStarting;

    unsigned long msitime;
    unsigned long msiwhen;    
};

#endif   //INVEIGLE_H
