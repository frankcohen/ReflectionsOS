#ifndef _COMPASS_
#define _COMPASS_

#include "config.h"
#include "secrets.h"

#include <Adafruit_MMC56x3.h>

#define maxCompassAngles 5    // How many running average angles to keep

class Compass
{
  public:
    Compass();
    void begin();
    void loop();
    String updateHeading();
    String getHeadings();
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
    long lastDisplayTime;

    String heading;
};

#endif // _COMPASS_
