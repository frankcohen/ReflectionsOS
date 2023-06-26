#ifndef _COMPASS_
#define _COMPASS_

#include "config.h"
#include "secrets.h"

#include <Adafruit_MMC56x3.h>

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
    void setHeading( String _heading );
    String getHeading();
    
  private:
    Adafruit_MMC5603 mag;
    boolean started;

    float MagMinX, MagMaxX; // For callibration
    float MagMinY, MagMaxY;
    float MagMinZ, MagMaxZ;
    long lastDisplayTime;

    String heading;
};

#endif // _COMPASS_
