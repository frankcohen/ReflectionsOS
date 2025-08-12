#pragma once
#include <Arduino.h>
#include "Experience.h"

// Forward declarations provided by ReflectionsOS
class Arduino_GFX;
class AccelSensor;
class TOF;
extern Arduino_GFX *gfx;
extern AccelSensor accel;
extern TOF tof;

class Experience_Sand : public Experience {
public:
  Experience_Sand();

  // Lifecycle exactly like Experience_Awake: ExperienceService calls these
  void setup() override;     // animate grains "falling in", then setSetupComplete(true)
  void run() override;       // gravity from accelerometer + TOF blocking zones
  void teardown() override;  // animate clearing, then setTeardownComplete(true)

private:
  // Screen
  static constexpr uint16_t W = 240;
  static constexpr uint16_t H = 240;

  // Particles
  static constexpr uint16_t MAX_PARTICLES   = 1600;  // conservative default
  static constexpr uint16_t SETUP_PARTICLES = 1100;

  // Physics (Q8.8)
  static constexpr int16_t  GRAVITY_SCALE_Q88 = 80;   // tilt gain
  static constexpr int16_t  FRICTION_Q88      = 252;  // ~0.985
  static constexpr int16_t  VEL_CAP_Q88       = 384;  // ~1.5 px/ms (~384/256)

  // TOF sampling (throttled) and zone sizing
  static constexpr uint8_t  MAX_ZONES     = 4;
  static constexpr uint16_t TOF_PERIOD_MS = 40;       // 25 Hz reads
  static constexpr uint16_t TOF_MIN_MM    = 180;
  static constexpr uint16_t TOF_MAX_MM    = 400;
  static constexpr uint8_t  ZONE_R_MIN    = 10;
  static constexpr uint8_t  ZONE_R_MAX    = 26;

  struct Zone {
    uint16_t cx, cy;
    uint8_t  r;
    uint32_t stamp;
    bool     active;
  };

  // Particle data
  uint16_t count_;
  uint16_t x_[MAX_PARTICLES];
  uint16_t y_[MAX_PARTICLES];
  int16_t  vx_[MAX_PARTICLES]; // Q8.8
  int16_t  vy_[MAX_PARTICLES]; // Q8.8
  uint8_t  live_[MAX_PARTICLES];

  // Zones
  Zone     zones_[MAX_ZONES];
  uint8_t  zoneCount_;

  // Timing
  uint32_t lastMs_;
  uint32_t lastTofMs_;

  // Cached TOF sample
  int8_t   lastCol_;
  int16_t  lastMm_;

  // Lifecycle guards (to match Experience_Awake style)
  bool     setupStarted_;
  uint32_t setupStartMs_;
  bool     teardownStarted_;
  uint32_t teardownStartMs_;

  // Helpers
  inline void putPixel(uint16_t px, uint16_t py, uint16_t color);
  inline void erasePixel(uint16_t px, uint16_t py) { putPixel(px,py,0x0000); }
  inline void drawPixel(uint16_t px, uint16_t py)  { putPixel(px,py,0xFFFF); }

  void seedInitial();
  void stepSetup(uint32_t dt);
  void stepRun(uint32_t dt);
  void stepTeardown(uint32_t dt);

  void readTilt(int16_t& gxQ, int16_t& gyQ);
  void applyOne(uint16_t i, int16_t axQ, int16_t ayQ);
  void integrate(uint16_t i, uint32_t dt);

  // TOF / zones
  void updateZonesFromTOF();
  void addOrUpdateZone(uint16_t sx, uint16_t sy, uint8_t r);
  bool  resolveBlocking(uint16_t &nx, uint16_t &ny, int16_t &vxQ, int16_t &vyQ);
};
