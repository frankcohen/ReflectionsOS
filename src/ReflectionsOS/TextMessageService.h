/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef _TEXTMESSAGESERVICE_
#define _TEXTMESSAGESERVICE_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"
#include "Video.h"
#include "RealTimeClock.h"

// Additional fonts at https://github.com/moononournation/ArduinoFreeFontFile/tree/master/Fonts
#include "SomeTimeLater20pt7b.h"
#include "SomeTimeLater12pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"
#include "Minya_Nouvelle_Rg30pt7b.h"

extern LOGGER logger;
extern Arduino_GFX *gfx;
extern RealTimeClock realtimeclock;

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

enum TextMessageExperiences 
{
    ShowDigitalTimeFunMessages,       // Shows time with funny message above and below
    MysticalAnswer,
    DigitalTime                       // Cat opens mouth to reveal the time
};

class TextMessageService
{
  public:
    TextMessageService();
    void begin();
    void loop();

    boolean fadeInCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font );
    boolean fadeOutCenteredText( String text, int16_t y, uint16_t duration, uint16_t color, uint16_t backcolor, const GFXfont * font );
    void startShow( int shownum, String msg1, String msg2 );

    bool active();
    void stop();
    void start();
    void deactivate();

    String getTimeStringFromAngle( float angle );

    bool isTimeSet();
    String getRTCtime();

    void updateTime();   // Redraws current time for wfMain
    void updateTempTime( String tempTime ); // Draws current time for wfMain set time panel
    void updateHealth( int smallsteps );
    void updateTimer( int minutesleft );

    void drawCenteredMesssage( String msg, String msg2 );

  private:
    void runShowDigitalTimeFunMessages();
    void runMysticShow();
    void runDigitalTime();

    String formatWithCommas(int value);

    int16_t x, y;
    uint16_t w, h;
    int fadestep;
    uint16_t steps;
    uint16_t stepDelay;
    int showNum;
    bool activated;
    long waitTime;
    bool fadeset;
    int showStep;
    
    String theMsg1;
    String theMsg2;

    struct tm timeinfo;
    String theTime;
};

#endif // _TEXTMESSAGESERVICE_
