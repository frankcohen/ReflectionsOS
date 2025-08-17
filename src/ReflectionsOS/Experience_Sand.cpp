/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

*/

#include "Experience_Sand.h"
#include "AccelSensor.h"
#include <math.h>

#ifndef BLACK
#define BLACK 0x0000
#endif
#ifndef WHITE
#define WHITE 0xFFFF
#endif

extern Arduino_GFX *gfx;
extern AccelSensor accel;

Experience_Sand::Experience_Sand() : ax0_(0.0f), ay0_(0.0f) {
  for (uint16_t i=0;i<MAX_PARTICLES;++i) live_[i]=0;
  for (uint8_t k=0;k<MAX_ISLANDS;++k) islands_[k] = { W/2, H/2, 0 };
}

void Experience_Sand::init() {
  setupComplete    = false;
  runComplete      = false;
  teardownComplete = false;
  stopped          = false;
  idle             = false;
}

void Experience_Sand::setup() {
  setExperienceName(sandname);

  Serial.print(sandname);
  Serial.println(F("SETUP"));

  ax0_ = 0.0f;
  ay0_ = 0.0f;
  for (uint16_t i=0;i<MAX_PARTICLES;++i) live_[i]=0;
  for (uint8_t k=0;k<MAX_ISLANDS;++k) islands_[k] = { W/2, H/2, 0 };

  if (gfx) gfx->fillScreen( COLOR_PANTONE_662 ); // deep blue background

  // Reset bubbles and seed inside the round screen
  for (uint16_t i=0;i<MAX_PARTICLES;++i) live_[i]=0;
  seedInitial();

  // Baseline accel like WatchFaceMain::settingtime()
  ax0_ = accel.getXreading();
  ay0_ = accel.getYreading();

  // Random islands (fit inside circle)
  chooseIslands();

  // Draw island sprites once at setup (they're static obstacles)
  if (DEBUG_DRAW) {
    drawIslandSprites();
  }

  sandtime = millis();
  dur      = random(1, 5);   // used below (35s .. 37.5s)
  pace     = millis();

  setSetupComplete(true);  // Signal that setup is complete
}

void Experience_Sand::run() {
  // pacing
  if (millis() - pace < 20) return;
  pace = millis();

  int16_t gxQ=0, gyQ=0; readTilt(gxQ, gyQ);

  // Physics with micro-substeps to stabilize motion
  uint32_t left = 10;  // simulate ~10ms worth per call
  while (left > 0) {
    uint32_t step = left > 16 ? 16 : left; // ≤ ~60Hz steps
    for (uint16_t i=0;i<MAX_PARTICLES;++i) if (live_[i]) {
      eraseBubble(x_[i], y_[i]);
      applyOne(i, gxQ, gyQ);
      integrate(i, step);
      deflectByIslands(i);
      drawBubble(x_[i], y_[i], color_[i]);
    }
    left -= step;
  }

  // ALWAYS draw island sprites last so they appear on top of bubbles
  drawIslandSprites();

  // End run after ~35s + dur*0.5s (your prior scheme)
  if (millis() - sandtime > (35000UL + (dur * 500UL))) {
    setRunComplete(true);
    return;
  }
}


void Experience_Sand::teardown() {
  Serial.print(sandname);
  Serial.println(F("TEARDOWN"));

  // Optional: clear immediately; keeping your current behavior
  // if (gfx) gfx->fillScreen(BLACK);

  setTeardownComplete(true);
}

// -------- basic helpers --------

inline void Experience_Sand::fillRectClipped(int x, int y, int w, int h, uint16_t c) {
  if (!gfx) return;
  if (x >= (int)W || y >= (int)H) return;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > (int)W) w = W - x;
  if (y + h > (int)H) h = H - y;
  if (w <= 0 || h <= 0) return;
  gfx->fillRect(x, y, w, h, c);
}

inline bool Experience_Sand::bubbleInsideCircle(int x, int y) const {
  // Use bubble center; shrink by radius to keep full bubble inside circle
  const int cxg = x + BUBBLE_R;
  const int cyg = y + BUBBLE_R;
  const int rr  = CR - BUBBLE_R;
  const int dx  = cxg - CX;
  const int dy  = cyg - CY;
  return (dx*dx + dy*dy) <= (rr*rr);
}

inline void Experience_Sand::drawBubble(uint16_t px, uint16_t py, uint16_t c) {
  if (!bubbleInsideCircle(px, py) || !gfx) return;

  // Skip drawing if bubble overlaps an island (so islands stay visually on top)
  const int cxg = (int)px + BUBBLE_R;
  const int cyg = (int)py + BUBBLE_R;
  for (uint8_t k=0; k<islandCount_; ++k) {
    const Island &Z = islands_[k];
    if (!Z.r) continue;
    const int dx = cxg - (int)Z.cx;
    const int dy = cyg - (int)Z.cy;
    const int rr = (int)Z.r + (int)BUBBLE_R - 1;
    if (dx*dx + dy*dy <= rr*rr) return;
  }

  gfx->fillCircle(px + BUBBLE_R, py + BUBBLE_R, BUBBLE_R, c);
  gfx->fillRect((int)px + 2, (int)py + 2, HILIGHT_SZ, HILIGHT_SZ, WHITE);
}


// -------- seeding & islands --------

void Experience_Sand::seedInitial() {
  // Scatter SETUP_PARTICLES with mild downward bias; draw them
  uint16_t placed = 0;
  while (placed < SETUP_PARTICLES) {
    // Find a free slot
    uint16_t i = 0;
    for (; i<MAX_PARTICLES; ++i) if (!live_[i]) break;
    if (i==MAX_PARTICLES) break;

    // Try a few positions until we get one inside the circle
    uint8_t tries = 0;
    do {
      x_[i] = random(0, W - BOX);
      y_[i] = random(0, H - BOX);
      tries++;
    } while (!bubbleInsideCircle(x_[i], y_[i]) && tries < 25);

    if (!bubbleInsideCircle(x_[i], y_[i])) continue;

    vx_[i]= 0;
    vy_[i]= 24; // slight downward for settling look
    live_[i]=1; ++count_;

    // Assign a random bubble base color from the 3-color palette
    color_[i] = PALETTE[random(0, (int)(sizeof(PALETTE)/sizeof(PALETTE[0])))];

    drawBubble(x_[i], y_[i], color_[i]);
    ++placed;
  }
}

void Experience_Sand::chooseIslands() {
  islandCount_ = 0;

  // 0 = small-only (1..3), 1 = large-only (1), 2 = medium + small (1 medium + 0..3 small)
  const uint8_t mode = (uint8_t)random(0, 3);

  auto addByKind = [&](IslandKind kind) -> bool {
    int16_t iw, ih;
    const char* _ = spriteForKind(kind, iw, ih); (void)_; // we only need size here
    uint8_t r = (kind == IK_Small ? SMALL_R : (kind == IK_Medium ? MED_R : LARGE_R));
    Island I;
    if (placeIslandSprite(r, iw, ih, I)) {
      if (islandCount_ < MAX_ISLANDS) {
        I.kind = (uint8_t)kind;
        islands_[islandCount_++] = I;
        return true;
      }
    }
    return false;
  };

  if (mode == 0) {
    // Small-only: 1..3 small
    const uint8_t target = (uint8_t)random(1, (uint8_t)min<int>(3, MAX_ISLANDS) + 1);
    uint8_t tries = 0;
    while (islandCount_ < target && tries < 80) { addByKind(IK_Small); ++tries; }
  } else if (mode == 1) {
    // Large-only: exactly 1 large
    for (uint8_t t=0; t<100 && islandCount_==0; ++t) { if (addByKind(IK_Large)) break; }
  } else {
    // Medium + Small: 1 medium + 0..3 small
    for (uint8_t t=0; t<100 && islandCount_==0; ++t) { if (addByKind(IK_Medium)) break; }
    const uint8_t moreSmall = (uint8_t)random(0, 4); // 0..3
    uint8_t tries = 0;
    while (islandCount_ < min<uint8_t>((uint8_t)(1 + moreSmall), MAX_ISLANDS) && tries < 120) {
      addByKind(IK_Small); ++tries;
    }
  }
}



// -------- physics --------

void Experience_Sand::readTilt(int16_t& gxQ, int16_t& gyQ) {
  const float ax = accel.getXreading();
  const float ay = accel.getYreading();

  auto mapf = [](float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - out_min) + out_min;
  };

  float gx = mapf(ax, ax0_ - TILT_RANGE, ax0_ + TILT_RANGE, -1.0f, 1.0f);
  float gy = mapf(ay, ay0_ - TILT_RANGE, ay0_ + TILT_RANGE, -1.0f, 1.0f);

  if (gx >  1.0f) gx =  1.0f; else if (gx < -1.0f) gx = -1.0f;
  if (gy >  1.0f) gy =  1.0f; else if (gy < -1.0f) gy = -1.0f;

  gxQ = (int16_t)(gx * 256.0f);
  gyQ = (int16_t)(gy * 256.0f);

  // Debug print (kept from your version)
  Serial.print("Experience_Sand ");
  Serial.print(gx);
  Serial.print(" ");
  Serial.print(gy);
  Serial.print(" ");
  Serial.print(gxQ);
  Serial.print(" ");
  Serial.println(gxQ);
}

void Experience_Sand::applyOne(uint16_t i, int16_t axQ, int16_t ayQ) {
  // gravity from tilt
  vx_[i] += (int16_t)((axQ * GRAVITY_SCALE_Q88) >> 8);
  vy_[i] += (int16_t)((ayQ * GRAVITY_SCALE_Q88) >> 8);

  // friction
  vx_[i] = (int16_t)((int32_t)vx_[i] * FRICTION_Q88 >> 8);
  vy_[i] = (int16_t)((int32_t)vy_[i] * FRICTION_Q88 >> 8);

  // clamp velocity hard
  if (vx_[i] >  VEL_CAP_Q88) vx_[i] =  VEL_CAP_Q88;
  if (vx_[i] < -VEL_CAP_Q88) vx_[i] = -VEL_CAP_Q88;
  if (vy_[i] >  VEL_CAP_Q88) vy_[i] =  VEL_CAP_Q88;
  if (vy_[i] < -VEL_CAP_Q88) vy_[i] = -VEL_CAP_Q88;
}

void Experience_Sand::integrate(uint16_t i, uint32_t dt) {
  int32_t nx = (int32_t)x_[i] + (((int32_t)vx_[i] * (int32_t)dt) >> 8);
  int32_t ny = (int32_t)y_[i] + (((int32_t)vy_[i] * (int32_t)dt) >> 8);

  // Update tentative position
  x_[i] = (uint16_t)nx;
  y_[i] = (uint16_t)ny;

  // Enforce circular screen boundary (bounce on rim)
  enforceCircleBoundary(i);
}

void Experience_Sand::enforceCircleBoundary(uint16_t i) {
  // Bubble center
  float cxg = (float)x_[i] + (float)BUBBLE_R;
  float cyg = (float)y_[i] + (float)BUBBLE_R;

  const float rr = (float)CR - (float)BUBBLE_R; // usable radius for full bubble
  const float dx = cxg - (float)CX;
  const float dy = cyg - (float)CY;
  const float d2 = dx*dx + dy*dy;

  if (d2 <= rr*rr) return; // already inside

  // Push back to rim and reflect velocity along outward normal
  float d = sqrtf(d2);
  float nx = 1.0f, ny = 0.0f;
  if (d >= 1e-3f) {
    nx = dx / d;
    ny = dy / d;
  }
  // Snap to rim
  cxg = (float)CX + nx * rr;
  cyg = (float)CY + ny * rr;

  // Reflect velocity
  float vxf = (float)vx_[i] / 256.0f;
  float vyf = (float)vy_[i] / 256.0f;
  float vn  = vxf * nx + vyf * ny;     // normal component
  vxf = vxf - 2.0f * vn * nx;
  vyf = vyf - 2.0f * vn * ny;

  // Mild damping on rim hit
  vxf *= 0.33f;
  vyf *= 0.33f;

  // Back to Q8.8
  vx_[i] = (int16_t)(vxf * 256.0f);
  vy_[i] = (int16_t)(vyf * 256.0f);

  // Write back bubble's top-left from center
  int px = (int)(cxg - (float)BUBBLE_R);
  int py = (int)(cyg - (float)BUBBLE_R);

  // Final clamp to on-screen (safety)
  if (px < 0) px = 0; else if (px > (int)W - BOX) px = (int)W - BOX;
  if (py < 0) py = 0; else if (py > (int)H - BOX) py = (int)H - BOX;

  x_[i] = (uint16_t)px;
  y_[i] = (uint16_t)py;
}

void Experience_Sand::deflectByIslands(uint16_t i) {
  // Bubble center for collision
  int32_t cxg = (int32_t)x_[i] + BUBBLE_R;
  int32_t cyg = (int32_t)y_[i] + BUBBLE_R;

  for (uint8_t k=0; k<islandCount_; ++k) {
    const Island &Z = islands_[k];
    if (!Z.r) continue;
    int32_t dx = cxg - (int32_t)Z.cx;
    int32_t dy = cyg - (int32_t)Z.cy;
    int32_t d2 = dx*dx + dy*dy;
    int32_t r2 = (int32_t)Z.r * (int32_t)Z.r;

    if (d2 < r2) {
      // Push to boundary along dominant axis & add a tangential nudge
      if (abs(dx) >= abs(dy)) {
        x_[i] = (dx > 0) ? (Z.cx + Z.r - BUBBLE_R) : (uint16_t)(Z.cx - Z.r - BUBBLE_R);
        vx_[i] = (int16_t)(-vx_[i] / 2);
        vy_[i] = (int16_t)( (vy_[i] * 3) / 4 + (dy >= 0 ? +64 : -64) );
      } else {
        y_[i] = (dy > 0) ? (Z.cy + Z.r - BUBBLE_R) : (uint16_t)(Z.cy - Z.r - BUBBLE_R);
        vy_[i] = (int16_t)(-vy_[i] / 2);
        vx_[i] = (int16_t)( (vx_[i] * 3) / 4 + (dx >= 0 ? +64 : -64) );
      }
    }
  }
}

// -------- phases --------

void Experience_Sand::stepSetup(uint32_t dt) {
  // Bias down in setup for “settling” look; caller handles pacing
  int16_t gxQ=0, gyQ=0; readTilt(gxQ, gyQ);
  gyQ += 24;

  for (uint16_t i=0;i<MAX_PARTICLES;++i) if (live_[i]) {
    eraseBubble(x_[i], y_[i]);
    applyOne(i, gxQ/2, gyQ);
    integrate(i, dt);
    deflectByIslands(i);
    drawBubble(x_[i], y_[i], color_[i]);
  }
}

void Experience_Sand::stepRun(uint32_t /*dt*/) {}
void Experience_Sand::stepTeardown(uint32_t /*dt*/) {}

// -------- PNG island sprites --------

const char* Experience_Sand::spriteForKind(uint8_t kind, int16_t& w, int16_t& h) const {
  // Use filenames only (your watchfacemain adds the folder path)
  switch (kind) {
    case IK_Small:  w = 61;  h = 66;  return "Sand_small.png";
    case IK_Medium: w = 124; h = 114; return "Sand_medium.png";
    default:        w = 164; h = 154; return "Sand_large.png";
  }
}

bool Experience_Sand::placeIslandSprite(uint8_t collisionR, int16_t iw, int16_t ih, Island &out) {
  // Half-diagonal of the PNG rectangle
  const float hw = iw * 0.5f, hh = ih * 0.5f;
  const int   cornerMargin = (int)ceilf(sqrtf(hw*hw + hh*hh));  // radius to furthest corner
  const int   maxCenterR   = (int)CR - cornerMargin;
  if (maxCenterR <= 0) return false; // sprite too big for the round screen

  for (uint8_t tries = 0; tries < 100; ++tries) {
    // Random center within allowed square bounds (then circle-filter)
    int16_t cx = (int16_t)random(cornerMargin, (int)W - cornerMargin);
    int16_t cy = (int16_t)random(cornerMargin, (int)H - cornerMargin);

    const int dxC = (int)cx - CX;
    const int dyC = (int)cy - CY;
    if (dxC*dxC + dyC*dyC > maxCenterR*maxCenterR) continue; // sprite would clip outside the circle

    // Non-overlap with existing islands
    bool ok = true;
    for (uint8_t k=0; k<islandCount_; ++k) {
      const Island &Z = islands_[k];
      const int dx = (int)cx - (int)Z.cx;
      const int dy = (int)cy - (int)Z.cy;
      const int rr = (int)collisionR + (int)Z.r;
      if (dx*dx + dy*dy < rr*rr) { ok = false; break; }
    }
    if (!ok) continue;

    out = { (uint16_t)cx, (uint16_t)cy, collisionR, 0 /*kind set by caller*/ };
    return true;
  }
  return false;
}

void Experience_Sand::drawIslandSprite(const Island& z) {
  int16_t iw, ih;
  const char* name = spriteForKind(z.kind, iw, ih);
  // Center the image on island center
  int16_t x = (int16_t)z.cx - iw/2;
  int16_t y = (int16_t)z.cy - ih/2;
  watchfacemain.drawImageFromFile(name, true, x, y);
}


void Experience_Sand::drawIslandSprites() {
  for (uint8_t k=0; k<islandCount_; ++k) {
    const Island &Z = islands_[k];
    if (!Z.r) continue;
    drawIslandSprite(Z);
  }

}
