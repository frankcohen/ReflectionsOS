#ifndef _COMPASS_
#define _COMPASS_

#include "config.h"
#include "secrets.h"

#include <Adafruit_MMC56x3.h>

#define maxCompassAngles 5    // How many running average angles to keep

#define MMC5603NJ_ID 0x00     // Internal ID for the magnetometer as found in the WHO_AM_I register
#define MMC5603NJ_ID_Alt 0x10 // Alternate for certain chips

class Compass
{
  public:
    Compass();
    void begin();
    void loop();
    String updateHeading();
    bool test();
    void callibrate();
    String decodeHeading( float measured_angle );
    float getHeading();
    void read_XYZ();
    
  private:
    Adafruit_MMC5603 mag;
    boolean started;

    float runningAngle[ maxCompassAngles ];

    //store highest, middle and lowest values, x component and y component
    float Max[2], Mid[2], Min[2], X, Y;

    String heading;

    unsigned long ctimer;
};

#endif // _COMPASS_