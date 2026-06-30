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

#define WAIT_TIME       250
#define SAMPLE_RATE     200       // Sample every x milliseconds

#define ACCEL_I2C_ADDR   LIS3DH_DEFAULT_ADDRESS   // 0x18 from Adafruit_LIS3DH

// ---- Deep-sleep wake tuning (LIS3DH click timing) ----
#define CLICKTHRESHHOLD      28    // much easier than 45/50/60
#define WAKE_MAX_WINDOW_MS   900   // slower, more natural double-tap
#define WAKE_MIN_MOTION_MS   30    // remove the “needs a whack” feel
#define WAKE_LATENCY_MS      10    // keep it responsive

#define SHAKEN_COUNT 2        // Big movements to signal shaken gesture

// -------- Wrist Twist Tunables --------
#define TWIST_MIN_ANGLE_DEG   95      // required total roll delta to trigger
#define TWIST_MAX_WINDOW_MS   450     // max time window for the twist
#define TWIST_COOLDOWN_MS     1200    // cooldown between twists
#define TWIST_G_MIN           9.0f    // minimum gravity magnitude
#define TWIST_G_MAX           10.8f   // maximum gravity magnitude
#define ROLL_EMA_ALPHA        0.15f   // smoothing for roll angle

// ===== Wrist Twist Debug & Options =====
#define TWIST_DEBUG           0     // 1 = print debug, 0 = silent
#define TWIST_AXIS_MODE       0     // 0: atan2(Y,Z)  (default/original)
                                    // 1: atan2(X,Z)  (use if board orientation differs)
#define TWIST_BYPASS_GRAVITY  0     // 1 = ignore gravity gate (self-test mode)
#define TWIST_DEBUG_PERIOD_MS 200   // min ms between debug lines

// -------- Set Time / Digital Time twist gesture tunables --------
#define SETTIME_TWIST_MOVE_EPS_DEG       1.3f
#define SETTIME_TWIST_START_MOVE_MS      180UL
#define SETTIME_TWIST_RETURN_MOVE_MS     180UL
#define SETTIME_TWIST_STOP_MS            200UL
#define SETTIME_TWIST_DIGITAL_DEG        80.0f
#define SETTIME_TWIST_REQUIRED_DEG       130.0f
#define SETTIME_TWIST_HOLD_MS            3000UL
#define SETTIME_TWIST_TRACK_TIMEOUT_MS   9000UL
#define SETTIME_TWIST_COOLDOWN_MS        3000UL
#define SETTIME_TWIST_NO_MOVE_TIMEOUT_MS 500UL

class AccelSensor
{
  public:
    AccelSensor();
    void begin();
    void loop();

    // Wake profile (call ONLY right before deep sleep)
    void configureSensorWakeOnMotion();  // existing wake configurator
    void configureWakeTapProfile();      // wrapper for clarity

    // Runtime profile (use while awake; less sensitive, Z-only)
    void configureRuntimeTapProfile();

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

    // Tap-to-abort helper:
    // - requires confirmed single tap
    // - requires Z-axis contribution
    // - requires stillness (reduces false abort while moving)
    bool getSingleTapAbortCandidateNoClear();
    bool getSingleTapAbortCandidate(); // consumes the pending single tap if it qualifies

    float getXreading();
    float getYreading();
    float getZreading();

    String getRecentMessage();
    String getRecentMessage2();

    bool getWristTwist();

    // Returns wrist twist direction:
    //  -1 = twist left (counter-clockwise)
    //  +1 = twist right (clockwise)
    //   0 = no significant twist
    int getWristTwistDir();

    // Higher-level twist gesture used by WatchFaceMain Set Time.
    // AccelSensor owns the accelerometer physics; WatchFaceMain owns UI meaning.
    bool isSetTimeTwisting();
    bool getSetTimeTwistDigitalTime();   // consumes event
    bool getSetTimeTwistSetTime();       // consumes event
    void resetSetTimeTwistGesture();
    void suppressSetTimeTwistFor(uint32_t ms);
    void printSetTimeTwistDebug();

  private:

    void handleClicks();
    void updateStillnessEstimate(float mag);

    Adafruit_LIS3DH lis;

    bool      started;
    bool      runflag;

    bool      firstpart;
    unsigned long firstClickTime;
    unsigned long minClickTime;
    unsigned long waittime;
    bool      _pendingSingle;
    bool      _pendingDouble;

    // Tap gating state (new)
    bool          _lastClickHadZ;
    unsigned long _lastClickMs;
    float         _magEma;
    float         _magDevEma;

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
    unsigned long _twistIgnoreUntilMs;

    unsigned long _twistDbgLastMs;

    float _ax, _ay, _az;
    int _twistDirHits;
    int _twistDirLast;

    enum SetTimeTwistState
    {
      SETTIME_TWIST_IDLE = 0,
      SETTIME_TWIST_FORWARD,
      SETTIME_TWIST_HOLDING,
      SETTIME_TWIST_RETURN,
      SETTIME_TWIST_COOLDOWN
    };

    void updateSetTimeTwistGesture(unsigned long now);
    float setTimeTwistAngleDeg_() const;
    static float shortestSetTimeTwistDelta_(float a_deg, float b_deg);

    SetTimeTwistState _setTimeTwistState;
    float _setTimeTwistIdleAngleDeg;
    float _setTimeTwistStartAngleDeg;
    float _setTimeTwistLastAngleDeg;
    int _setTimeTwistForwardDir;
    bool _setTimeTwistReachedDigital;
    bool _setTimeTwistReturnConfirmed;
    unsigned long _setTimeTwistStartedAt;
    unsigned long _setTimeTwistHoldStartedAt;
    unsigned long _setTimeTwistCooldownUntil;
    unsigned long _setTimeTwistForwardMoveStartedAt;
    unsigned long _setTimeTwistReturnMoveStartedAt;
    unsigned long _setTimeTwistStoppedStartedAt;
    bool _setTimeTwistDigitalTimePending;
    bool _setTimeTwistSetTimePending;
    unsigned long _setTimeTwistNoMoveStartedAt;
};

#endif // ACCEL_SENSOR_H