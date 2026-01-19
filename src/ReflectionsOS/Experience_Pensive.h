#pragma once

#include <Arduino.h>
#include <math.h>

#include "Experience.h"
#include "Video.h"
#include "WatchFaceMain.h"
#include "AccelSensor.h"

extern Arduino_GFX *gfx;
extern WatchFaceMain watchfacemain;   // for drawImageFromFile()
extern AccelSensor accel;
extern Video video;

#define pensiveName F("Pensive ")

class Experience_Pensive : public Experience
{
public:
  Experience_Pensive();

  void init() override;
  void setup() override;
  void run() override;
  void teardown() override;

private:
  // Screen geometry
  static constexpr int   kW  = 240;
  static constexpr int   kH  = 240;
  static constexpr int   kCX = 120;
  static constexpr int   kCY = 120;

  // Timing (auto-exit)
  static constexpr uint32_t kDurationMs = 20000; // 20s
  uint32_t startMs_ = 0;

  // Particles
  static constexpr int   kMaxP = 48;

  // Physics
  static constexpr float kDrag        = 0.985f;
  static constexpr float kBaseG       = 0.38f;
  static constexpr float kSoftening   = 90.0f;
  static constexpr float kSpawnRadius = 110.0f;
  static constexpr float kKillRadius  = 10.0f;
  static constexpr float kMaxSpeed    = 5.0f;

  // Tilt-driven well
  static constexpr float kWellMaxOffset = 55.0f;
  static constexpr float kWellSmooth    = 0.07f;

  // Dead-zone (prevents drift)
  static constexpr float kTiltDeadZone  = 0.06f;  // normalized 0..1

  // Swirl changes with tilt (direction + strength)
  static constexpr float kSwirlBase     = 0.55f;
  static constexpr float kSwirlTiltGain = 0.55f;
  static constexpr float kSwirlDirBias  = 0.85f;

  // Render
  static constexpr int kParticleSize = 2;
  static constexpr uint16_t kBg = 0x0000; // black

  // Vortex core sprite (from /REFLECTIONS/Sand_small.png)
  static constexpr const char* kCorePng = "Sand_small.png";
  static constexpr int16_t kCoreW = 61;
  static constexpr int16_t kCoreH = 66;

  // Color ramp stops (RGB565)
  // #FFEC2D, #A9C47F, #041E42, #5BD0E6, #1E5CA1
  static constexpr uint16_t kRamp[5] = { 0xFF65, 0xAE2F, 0x00E8, 0x5E9C, 0x1AF4 };

  struct P
  {
    float x, y;
    float vx, vy;
    float mass;
    float alpha;
    uint32_t seed;

    uint32_t ageMs;   // lifetime progress
    uint32_t ttlMs;   // lifetime total (ms)
    uint16_t color;   // computed each frame from ramp
  };

  P p_[kMaxP];

  float wellX_ = kCX;
  float wellY_ = kCY;
  float targetWellX_ = kCX;
  float targetWellY_ = kCY;

  float tiltX_ = 0.0f;
  float tiltY_ = 0.0f;
  float tiltMag_ = 0.0f;

  uint32_t lastMs_ = 0;

private:
  void seedParticles_();
  void spawnOne_(int i);

  void readTilt_();
  void updateWellFromTilt_();

  void stepParticles_(uint32_t dms, float dt);
  void render_();

  // helpers
  static float clampf_(float v, float lo, float hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }
  static float fastLen_(float x, float y) { return sqrtf(x*x + y*y); }
  static float applyDeadZone_(float v, float dz);

  static float rand01_(uint32_t &s);
  static uint16_t lerp565_(uint16_t a, uint16_t b, float t);

  // multi-stop ramp color
  static uint16_t ramp565_(float t01);
};