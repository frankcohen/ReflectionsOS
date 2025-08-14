#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "Experience.h"
#include "config.h"  // Pantone color defines
#include "WatchFaceMain.h"

// Forward decls from your project
class Arduino_GFX;
class AccelSensor;
extern Arduino_GFX *gfx;
extern AccelSensor accel;
extern WatchFaceMain watchfacemain;

#define sandname F("Sand ")

class Experience_Sand : public Experience {
public:
  Experience_Sand();

  // Lifecycle (called by ExperienceService)
  void init() override;
  void setup() override;     // seed bubbles; choose islands
  void run() override;       // tilt gravity + island deflection; setRunComplete after time window
  void teardown() override;  // signal teardown complete

private:
  // Tight mapping to art (visual diameters)
  static constexpr uint8_t SMALL_R = 35;   // 70 px diameter
  static constexpr uint8_t MED_R   = 65;   // 130 px diameter
  static constexpr uint8_t LARGE_R = 85;   // 170 px diameter

  // Screen
  static constexpr uint16_t W  = 240;
  static constexpr uint16_t H  = 240;

  // Round display geometry (centered 240x240 circle)
  static constexpr int16_t  CX = 120;
  static constexpr int16_t  CY = 120;
  static constexpr int16_t  CR = 120;   // radius of circle

  // Debug toggle
  static constexpr bool DEBUG_DRAW = true;   // draw PNG islands

  // Bubble geometry
  static constexpr uint8_t  BOX = 10;        // placement box; bubble diameter = 10
  static constexpr uint8_t  BUBBLE_R = 5;    // bubble radius
  static constexpr uint8_t  HILIGHT_SZ = 2;  // 2x2 highlight

  // Particles / physics (Q8.8)
  static constexpr uint16_t MAX_PARTICLES     = 400;
  static constexpr uint16_t SETUP_PARTICLES   = 400;
  static constexpr int16_t  GRAVITY_SCALE_Q88 = 80;
  static constexpr int16_t  FRICTION_Q88      = 252;  // ~0.985
  static constexpr int16_t  VEL_CAP_Q88       = 384;  // ~1.5 px/ms

  // Tilt mapping baseline (like WatchFaceMain::settingtime())
  float   ax0_, ay0_;                         // baseline captured in setup()
  static constexpr float TILT_RANGE = 0.35f;  // ±g maps to [-1..+1]

  // Bubble palette (only these three)
  static constexpr uint16_t PALETTE[3] = {
    COLOR_PANTONE_102, // yellow
    COLOR_PANTONE_310, // light cyan
    COLOR_PANTONE_313  // cyan
  };

  // Islands
  static constexpr uint8_t  MAX_ISLANDS  = 4;
  static constexpr uint8_t  ISLAND_R_MIN = 14;
  static constexpr uint8_t  ISLAND_R_MAX = 28;

  enum IslandKind : uint8_t { IK_Small = 0, IK_Medium = 1, IK_Large = 2 };

  struct Island { uint16_t cx, cy; uint8_t r; uint8_t kind; };

  Island   islands_[MAX_ISLANDS];
  uint8_t  islandCount_ = 0;

  // Particles
  uint16_t count_{0};
  uint16_t x_[MAX_PARTICLES]{};  // top-left of 10x10 bubble box
  uint16_t y_[MAX_PARTICLES]{};
  int16_t  vx_[MAX_PARTICLES]{}; // Q8.8
  int16_t  vy_[MAX_PARTICLES]{}; // Q8.8
  uint8_t  live_[MAX_PARTICLES]{};
  uint16_t color_[MAX_PARTICLES]{};

  // Timing (your current scheme)
  unsigned long sandtime{0};
  unsigned long dur{0};
  unsigned long pace{0};

  // Helpers
  inline void fillRectClipped(int x, int y, int w, int h, uint16_t c);
  inline bool bubbleInsideCircle(int x, int y) const;
  inline void eraseBubble(uint16_t px, uint16_t py) {
    if (!bubbleInsideCircle(px, py) || !gfx) return;
    gfx->fillCircle( px + BUBBLE_R, py + BUBBLE_R, BUBBLE_R, COLOR_PANTONE_662 );
  }
  inline void drawBubble(uint16_t px, uint16_t py, uint16_t c);

  void seedInitial();
  void chooseIslands();                // 1..4 non-overlapping (may touch)
  bool placeIsland(uint16_t r, Island &out);

  void stepSetup(uint32_t dt);
  void stepRun(uint32_t dt);
  void stepTeardown(uint32_t dt);      // unused in current flow

  void readTilt(int16_t& gxQ, int16_t& gyQ);
  void applyOne(uint16_t i, int16_t axQ, int16_t ayQ);
  void integrate(uint16_t i, uint32_t dt);
  void deflectByIslands(uint16_t i);   // integer deflection
  void enforceCircleBoundary(uint16_t i);

  // PNG island sprites
  void drawIslandSprites();
  bool placeIslandSprite(uint8_t collisionR, int16_t iw, int16_t ih, Island &out);
  void drawIslandSprite(const Island& z);
  const char* spriteForKind(uint8_t kind, int16_t& w, int16_t& h) const;
};
