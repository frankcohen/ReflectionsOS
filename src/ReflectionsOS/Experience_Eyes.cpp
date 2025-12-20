/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Experience: EyesFollowFinger
 (TOF-driven pupils with velocity, gravity, and edge damping)
*/

#include "Experience_Eyes.h"
#include <math.h>

// ------------------------------------------------------------
// DEBUG TOGGLES
// ------------------------------------------------------------
#define EYES_DEBUG_SERIAL  0   // 1 = enable Serial debug
#define EYES_DEBUG_DOT        // 1 = enable on-screen debug dot

#define COLOR_DEBUG 0xF800

// ------------------------------------------------------------
// PUPIL CONSTRAINTS
// ------------------------------------------------------------
static constexpr int LEFT_MIN_X  = 60;
static constexpr int LEFT_MAX_X  = 100;

static constexpr int RIGHT_MIN_X = 160;
static constexpr int RIGHT_MAX_X = 200;

// ------------------------------------------------------------
// ANIMATION TUNABLES
// ------------------------------------------------------------
static constexpr float DT_SEC               = 1.0f / 30.0f;
static constexpr float SPRING_K             = 38.0f;
static constexpr float BASE_DAMPING         = 8.0f;
static constexpr float GRAVITY_GAIN         = 32.0f;
static constexpr float EDGE_SLOW_ZONE_PX    = 10.0f;
static constexpr float EDGE_DAMPING_BOOST   = 28.0f;
static constexpr float MAX_SPEED_PX_PER_SEC = 520.0f;

// ------------------------------------------------------------
// INTERNAL STATE (file-local)
// ------------------------------------------------------------
static bool  s_animInit     = false;
static float s_leftX        = 80.0f;
static float s_rightX       = 180.0f;
static float s_leftV        = 0.0f;
static float s_rightV       = 0.0f;
static int   s_prevTofCol   = -1;

// ------------------------------------------------------------
// HELPERS
// ------------------------------------------------------------
static float clampf(float v, float lo, float hi)
{
  return (v < lo) ? lo : (v > hi) ? hi : v;
}

static float edgeDampingFor(float x, float lo, float hi)
{
  float dLeft  = x - lo;
  float dRight = hi - x;
  float dMin   = (dLeft < dRight) ? dLeft : dRight;

  float t = 0.0f;
  if (dMin < EDGE_SLOW_ZONE_PX)
    t = (EDGE_SLOW_ZONE_PX - dMin) / EDGE_SLOW_ZONE_PX;

  return EDGE_DAMPING_BOOST * t;
}

static float applySpeedClamp(float v)
{
  if (v >  MAX_SPEED_PX_PER_SEC) return  MAX_SPEED_PX_PER_SEC;
  if (v < -MAX_SPEED_PX_PER_SEC) return -MAX_SPEED_PX_PER_SEC;
  return v;
}

// ------------------------------------------------------------

void Experience_Eyes::init()
{
  setupComplete = false;
  runComplete = false;
  teardownComplete = false;
  vidflag = true;
  stopped = false;
  idle = false;

  s_animInit = false;
  s_prevTofCol = -1;

  prevLeftPupilX  = -1;
  prevRightPupilX = -1;
}

float Experience_Eyes::mapFloat(float x, float in_min, float in_max,
                                float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Experience_Eyes::setup()
{
  if (vidflag)
  {
    setExperienceName(eyesname);
    video.startVideo(EyesFollowFinger_video);

    vidflag = false;
    tearflag = true;
    pace = millis();
  }

  if (video.getVideoTime() > 2800)
  {
    eyestime = millis();
    dur = random(1, 5);
    video.setPaused(true);
    setSetupComplete(true);
  }
}

void Experience_Eyes::run()
{
  if (millis() - eyestime > (15000 + (dur * 250)))
  {
    setRunComplete(true);
    video.setPaused(false);
    return;
  }

  // ~30 FPS
  const unsigned long FRAME_MS = 33;
  if (millis() - pace < FRAME_MS) return;
  pace = millis();

  // ---------------- TOF INPUT ----------------
  int   tofCol  = tof.getFingerPos();    // 0..7
  float tofDist = tof.getFingerDist();

  bool tofPresent = (tofDist >= MIN_DIST_MM && tofDist <= MAX_DIST_MM);

  if (!s_animInit)
  {
    s_leftX  = (LEFT_MIN_X  + LEFT_MAX_X)  * 0.5f;
    s_rightX = (RIGHT_MIN_X + RIGHT_MAX_X) * 0.5f;
    s_leftV = s_rightV = 0.0f;

    s_prevTofCol = constrain(tofCol, 0, 7);
    s_animInit = true;
  }

  int tcol = constrain(tofCol, 0, 7);

  // ---------------- GRAVITY TREND ----------------
  int dcol = (s_prevTofCol == -1) ? 0 : (tcol - s_prevTofCol);
  s_prevTofCol = tcol;

  float gravDir = (dcol > 0) ? 1.0f : (dcol < 0) ? -1.0f : 0.0f;

  // ---------------- RANGE STRETCH ----------------
  // Treat practical 0..5 as full travel
  float tcolExpanded = mapFloat((float)tcol, 0.0f, 5.0f, 0.0f, 7.0f);
  tcolExpanded = clampf(tcolExpanded, 0.0f, 7.0f);

  float targetLeftX  = mapFloat(tcolExpanded, 0.0f, 7.0f,
                                (float)LEFT_MIN_X, (float)LEFT_MAX_X);

  float targetRightX = mapFloat(tcolExpanded, 0.0f, 7.0f,
                                (float)RIGHT_MIN_X, (float)RIGHT_MAX_X);

  // ---------------- PHYSICS ----------------
  auto stepOne = [&](float &x, float &v, float target, float lo, float hi)
  {
    float a = SPRING_K * (target - x);
    a += GRAVITY_GAIN * gravDir;
    a += -(BASE_DAMPING + edgeDampingFor(x, lo, hi)) * v;

    v += a * DT_SEC;
    v = applySpeedClamp(v);
    x += v * DT_SEC;

    if (x < lo) { x = lo; if (v < 0) v = 0; }
    if (x > hi) { x = hi; if (v > 0) v = 0; }
  };

  stepOne(s_leftX,  s_leftV,  targetLeftX,
          (float)LEFT_MIN_X,  (float)LEFT_MAX_X);

  stepOne(s_rightX, s_rightV, targetRightX,
          (float)RIGHT_MIN_X, (float)RIGHT_MAX_X);

  int leftPupilX  = (int)lroundf(s_leftX);
  int rightPupilX = (int)lroundf(s_rightX);

  // ---------------- DRAW ----------------
  if (leftPupilX != prevLeftPupilX || rightPupilX != prevRightPupilX)
  {
    if (prevLeftPupilX != -1)
    {
      gfx->fillCircle(prevLeftPupilX,  100, 18, COLOR_EYES_LEFT);
      gfx->fillCircle(prevRightPupilX, 100, 18, COLOR_EYES_RIGHT);
    }

    gfx->fillCircle(leftPupilX,  100, 14, COLOR_PUPILS);
    gfx->fillCircle(rightPupilX, 100, 14, COLOR_PUPILS);

    prevLeftPupilX  = leftPupilX;
    prevRightPupilX = rightPupilX;
  }

#if EYES_DEBUG_DOT
  // ---------------- DEBUG DOT ----------------
  {
    static int prevDbgX = -1;
    if (prevDbgX != -1)
      gfx->fillCircle(prevDbgX, 140, 4, 0x0000);

    if (tofPresent)
    {
      int dbgX = (int)lroundf(
        mapFloat(tcolExpanded, 0.0f, 7.0f,
                 (float)LEFT_MIN_X, (float)RIGHT_MAX_X)
      );
      gfx->fillCircle(dbgX, 140, 3, COLOR_DEBUG);
      prevDbgX = dbgX;
    }
    else prevDbgX = -1;
  }
#endif

#if EYES_DEBUG_SERIAL
  // ---------------- SERIAL DEBUG ----------------
  {
    static unsigned long dbgTimer = 0;
    if (millis() - dbgTimer > 100)
    {
      dbgTimer = millis();

      Serial.print("Eyes TOFcol=");
      Serial.print(tofCol);
      Serial.print(" dist=");
      Serial.print(tofDist, 1);
      Serial.print(" dcol=");
      Serial.print(dcol);
      Serial.print(" tExp=");
      Serial.print(tcolExpanded, 2);
      Serial.print(" targetL=");
      Serial.print(targetLeftX, 1);
      Serial.print(" xL=");
      Serial.print(s_leftX, 1);
      Serial.print(" vL=");
      Serial.print(s_leftV, 1);
      Serial.print(" targetR=");
      Serial.print(targetRightX, 1);
      Serial.print(" xR=");
      Serial.print(s_rightX, 1);
      Serial.print(" vR=");
      Serial.println(s_rightV, 1);
    }
  }
#endif
}

void Experience_Eyes::teardown()
{
  if (tearflag)
  {
    tearflag = false;
    video.setPaused(false);
  }

  if (!video.getStatus())
  {
    setTeardownComplete(true);
  }
}