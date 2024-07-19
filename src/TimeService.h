#ifndef _TIMESERVICE_
#define _TIMESERVICE_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Battery.h"
#include "GPS.h"
#include "Accelerometer.h"
#include "Video.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts

#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

extern const char* pairs[][2];

// Define color and geometry constants
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define CENTER_X 120
#define CENTER_Y 120
#define RADIUS 110
#define CARROT_RADIUS 10
#define CARROT_MOVEMENT_RADIUS 80 // Reduced radius so it doesn't overwrite the dial face
#define SETTIME_TIMEOUT 7000
#define SNAP_ANGLE_INCREMENT 7.5 // Increment for angle movement, smaller value for fine adjustment

class TimeService
{
  public:
    TimeService();
    void begin();
    void loop();
    bool test();

    boolean fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, const GFXfont * font);
    boolean fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, const GFXfont * font);
    void startShow( int shownum );

    void setDialActivated( bool set );
    bool getDialActivated();

    bool getTimeAnimationActivated();
    void setTimeAnimationActivated( bool act );

    String getTimeStringFromAngle( float angle );

    bool isTimeSet();
    String getRTCtime();

  private:
    void runShowTellTime();
    void updateSetTime();

    void drawClockFace();
    void showDigitalTime();
    void drawCarrot( float angle );
    void getTimeFromAngle(float angle, int &hour, int &minute);
    void setCarrotAngleFromRTC();

    int16_t x, y;
    uint16_t w, h;
    int fadestep;
    uint16_t steps;
    uint16_t stepDelay;
    int showNum;
    bool activated;
    long ShowTimeWaitTime;
    bool fadeset;
    int showStep;
    String theMsg1;
    String theMsg2;

    bool timeValid;
    struct tm timeinfo;
    String theTime;

    float carrotAngle;
    unsigned long lastMoveTime;
    bool moving;
    bool prior;

    bool dialActivated;

    unsigned long stTime;
    bool timeShowing;

    String pastTimeStr;
};

#endif // _TIMESERVICE_
