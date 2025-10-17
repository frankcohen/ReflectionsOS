/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef ACCEL_SENSOR_H
#define ACCEL_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "secrets.h"

#include "Utils.h"
#include "Logger.h"

#include "Video.h"

extern Utils utils;
extern LOGGER logger;
extern Video video;

#define WAIT_TIME       2000
#define SAMPLE_RATE     200       // Sample every x milliseconds

#define ACCEL_I2C_ADDR   LIS3DH_DEFAULT_ADDRESS   // 0x18 from Adafruit_LIS3DH

#define CLICKTHRESHHOLD 40    // Strenth of tap to bring back from deep sleep

#define SHAKEN_COUNT 2        // Big movements to signal shaken gesture

// -------- Wrist Twist Tunables --------
#define TWIST_MIN_ANGLE_DEG   75      // required total roll delta to trigger
#define TWIST_MAX_WINDOW_MS   600     // max time window for the twist
#define TWIST_COOLDOWN_MS     1200    // cooldown between twists
#define TWIST_G_MIN           8.0f    // minimum gravity magnitude
#define TWIST_G_MAX           12.5f   // maximum gravity magnitude
#define ROLL_EMA_ALPHA        0.25f   // smoothing for roll angle

// ===== Wrist Twist Debug & Options =====
#define TWIST_DEBUG           0     // 1 = print debug, 0 = silent
#define TWIST_AXIS_MODE       0     // 0: atan2(Y,Z)  (default/original)
                                    // 1: atan2(X,Z)  (use if board orientation differs)
#define TWIST_BYPASS_GRAVITY  0     // 1 = ignore gravity gate (self-test mode)
#define TWIST_DEBUG_PERIOD_MS 200   // min ms between debug lines

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    void configureSensorWakeOnMotion();

    void setStatus( bool running );
    bool getStatus();

    bool isShaken();

    /**
     * Returns true once when a confirmed single tap is detected.
     * Confirmation occurs after the double-tap window elapses without a second tap.
     */
    bool getSingleTap();

    /**
     * Returns true once when a confirmed double tap is detected.
     */
    bool getDoubleTap();

    void resetTaps();
    bool getSingleTapNoClear();
    bool getDoubleTapNoClear();
    
    float getXreading();
    float getYreading();
    float getZreading();
  
    String getRecentMessage();
    String getRecentMessage2();

    bool getWristTwist();

  private:

    void handleClicks();
    
    Adafruit_LIS3DH lis;

    bool      started;
    bool      runflag;

    bool      firstpart;
    unsigned long firstClickTime;
    unsigned long minClickTime;
    unsigned long waittime;
    bool      _pendingSingle;
    bool      _pendingDouble;

    int shakencount;
    unsigned long shakentime;
    unsigned long shakentime2;

    unsigned long magtime;
    float magnow;

    unsigned long last;        
    String    myMef;
    String    myMef2;

    // ---- Wrist Twist Detection State ----
    float         _rollEma;               // smoothed roll angle
    float         _lastRoll;              // previous roll
    float         _twistAccumDeg;         // accumulated roll delta
    unsigned long _twistStartMs;          // window start
    unsigned long _twistLastMs;           // last twist update
    unsigned long _twistCooldownUntil;    // cooldown until this time
    bool          _twistArmed;            // currently tracking a twist
    bool          _twistPending;          // true if a twist is detected

    unsigned long _twistDbgLastMs;
};

#endif // ACCEL_SENSOR_H
