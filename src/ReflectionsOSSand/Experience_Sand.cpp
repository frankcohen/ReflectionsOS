#include "Experience_Sand.h"
#include "AccelSensor.h"
#include "TOF.h"

#ifndef BLACK
#define BLACK 0x0000
#endif
#ifndef WHITE
#define WHITE 0xFFFF
#endif

extern Arduino_GFX *gfx;
extern AccelSensor accel;
extern TOF tof;

Experience_Sand::Experience_Sand()
: count_(0),
  zoneCount_(0),
  lastMs_(0),
  lastTofMs_(0),
  lastCol_(-1),
  lastMm_(-1)
{
  for (uint16_t i=0;i<MAX_PARTICLES;++i) live_[i]=0;
  for (uint8_t z=0; z<MAX_ZONES; ++z) zones_[z] = {120,120,0,0,false};
}

void Experience_Sand::setup() {

  setExperienceName(F("Sand ")); // keep trailing space per your codebase
  if (gfx) gfx->fillScreen(BLACK);

  count_ = 0;
  for (uint16_t i=0;i<MAX_PARTICLES;++i) live_[i]=0;
  zoneCount_ = 0;
  lastMs_ = millis();
  lastTofMs_ = 0;
  lastCol_ = -1;
  lastMm_ = -1;

  seedInitial(); // animate drop-in during first few frames inside run()
}

void Experience_Sand::run() {
  const uint32_t now = millis();
  uint32_t dt = now - lastMs_;
  if (dt > 33) dt = 33; // clamp for stability
  lastMs_ = now;

  // 1) Setup phase animation for a short while (grains fall into place)
  stepSetup(dt); // light bias + gravity; will quickly resemble settled state

  // 2) Main sim
  stepRun(dt);
}

void Experience_Sand::teardown() {
  const uint32_t now = millis();
  uint32_t dt = now - lastMs_;
  if (dt > 33) dt = 33;
  lastMs_ = now;

  stepTeardown(dt);
}

/* ---------- helpers ---------- */

inline void Experience_Sand::putPixel(uint16_t px, uint16_t py, uint16_t color) {
  if (!gfx) return;
  if (px >= W || py >= H) return;
  gfx->drawPixel(px, py, color);
}

void Experience_Sand::seedInitial() {
  // Scatter SETUP_PARTICLES with mild downward bias; draw them
  uint16_t placed = 0;
  while (placed < SETUP_PARTICLES) {
    // Find a free slot
    uint16_t i = 0;
    for (; i<MAX_PARTICLES; ++i) if (!live_[i]) break;
    if (i==MAX_PARTICLES) break;

    x_[i] = random(0, W);
    y_[i] = random(0, H);
    vx_[i]= 0;
    vy_[i]= 24; // slight downward
    live_[i]=1; ++count_;
    drawPixel(x_[i], y_[i]);
    ++placed;
  }
}

void Experience_Sand::readTilt(int16_t& gxQ, int16_t& gyQ) {
  // Map accel to screen axes; +x right, +y down
  const float ax = accel.getXreading();
  const float ay = accel.getYreading();
  gxQ = (int16_t)(ax * 256.0f);
  gyQ = (int16_t)(ay * 256.0f);
}

void Experience_Sand::applyOne(uint16_t i, int16_t axQ, int16_t ayQ) {
  // gravity from tilt
  vx_[i] += (int16_t)((axQ * GRAVITY_SCALE_Q88) >> 8);
  vy_[i] += (int16_t)((ayQ * GRAVITY_SCALE_Q88) >> 8);

  // friction
  vx_[i] = (int16_t)((int32_t)vx_[i] * FRICTION_Q88 >> 8);
  vy_[i] = (int16_t)((int32_t)vy_[i] * FRICTION_Q88 >> 8);

  // NaN guards (just in case)
  if (vx_[i] != vx_[i]) vx_[i] = 0;
  if (vy_[i] != vy_[i]) vy_[i] = 0;

  // clamp velocity
  if (vx_[i] >  VEL_CAP_Q88) vx_[i] =  VEL_CAP_Q88;
  if (vx_[i] < -VEL_CAP_Q88) vx_[i] = -VEL_CAP_Q88;
  if (vy_[i] >  VEL_CAP_Q88) vy_[i] =  VEL_CAP_Q88;
  if (vy_[i] < -VEL_CAP_Q88) vy_[i] = -VEL_CAP_Q88;
}

void Experience_Sand::integrate(uint16_t i, uint32_t dt) {
  int32_t nx = (int32_t)x_[i] + (((int32_t)vx_[i] * (int32_t)dt) >> 8);
  int32_t ny = (int32_t)y_[i] + (((int32_t)vy_[i] * (int32_t)dt) >> 8);

  if (nx < 0)            { nx=0;      vx_[i] = -vx_[i]/3; }
  else if (nx > (W-1))   { nx=W-1;    vx_[i] = -vx_[i]/3; }
  if (ny < 0)            { ny=0;      vy_[i] = -vy_[i]/3; }
  else if (ny > (H-1))   { ny=H-1;    vy_[i] = -vy_[i]/3; }

  x_[i] = (uint16_t)nx;
  y_[i] = (uint16_t)ny;
}

/* ---------- phases ---------- */

void Experience_Sand::stepSetup(uint32_t dt) {
  // gentle fall-in for existing live particles
  int16_t gxQ=0, gyQ=0;
  readTilt(gxQ, gyQ);
  // bias down in setup to look like sand settling
  gyQ += 24;

  for (uint16_t i=0;i<MAX_PARTICLES;++i) if (live_[i]) {
    erasePixel(x_[i], y_[i]);
    applyOne(i, gxQ/2, gyQ);
    integrate(i, dt);
    drawPixel(x_[i], y_[i]);
  }
}

void Experience_Sand::stepRun(uint32_t dt) {
  // throttle TOF (25 Hz), cache last reading
  updateZonesFromTOF();

  int16_t gxQ=0, gyQ=0;
  readTilt(gxQ, gyQ);

  for (uint16_t i=0;i<MAX_PARTICLES;++i) if (live_[i]) {
    erasePixel(x_[i], y_[i]);
    applyOne(i, gxQ, gyQ);
    integrate(i, dt);
    (void)resolveBlocking(x_[i], y_[i], vx_[i], vy_[i]); // integer-only, safe
    drawPixel(x_[i], y_[i]);
  }
}

void Experience_Sand::stepTeardown(uint32_t dt) {
  // lift upward until off-screen, then stop drawing
  for (uint16_t i=0;i<MAX_PARTICLES;++i) if (live_[i]) {
    erasePixel(x_[i], y_[i]);
    // strong upward pull
    applyOne(i, 0, -160);
    integrate(i, dt);
    if (y_[i] == 0) {
      live_[i]=0;
      if (count_>0) --count_;
    } else {
      drawPixel(x_[i], y_[i]);
    }
  }
}

/* ---------- TOF & zones ---------- */

void Experience_Sand::updateZonesFromTOF() {
  const uint32_t now = millis();
  if (now - lastTofMs_ < TOF_PERIOD_MS) {
    // reuse cached sample
  } else {
    lastTofMs_ = now;
    const int col = tof.getFingerPos();   // 0..7, 0/neg == none
    const float mmf = tof.getFingerDist();
    // cache (store ints)
    lastCol_ = (col <= 0) ? -1 : (int8_t)col;
    lastMm_  = (int16_t)((mmf < 0) ? -1 : mmf);
  }

  // mark all inactive; we'll reactivate touched one
  for (uint8_t z=0; z<zoneCount_; ++z) zones_[z].active = false;

  if (lastCol_ < 0 || lastMm_ <= 0) return;

  uint16_t sx = (uint16_t)((lastCol_ + 0.5f) * (W / 8.0f));
  uint16_t sy = H/2;

  int mm = lastMm_;
  if (mm < (int)TOF_MIN_MM) mm = TOF_MIN_MM;
  if (mm > (int)TOF_MAX_MM) mm = TOF_MAX_MM;
  const uint8_t r = (uint8_t)(ZONE_R_MAX - (uint32_t)(mm - TOF_MIN_MM) * (ZONE_R_MAX - ZONE_R_MIN) / (TOF_MAX_MM - TOF_MIN_MM));

  addOrUpdateZone(sx, sy, r);
}

void Experience_Sand::addOrUpdateZone(uint16_t sx, uint16_t sy, uint8_t r) {
  // merge into nearest existing zone (within 0.75 r)
  uint8_t best = 255;
  uint32_t bestd = 0xFFFFFFFF;
  for (uint8_t z=0; z<zoneCount_; ++z) {
    int32_t dx = (int32_t)sx - zones_[z].cx;
    int32_t dy = (int32_t)sy - zones_[z].cy;
    uint32_t d2 = (uint32_t)(dx*dx + dy*dy);
    uint32_t th = (uint32_t)zones_[z].r * (uint32_t)zones_[z].r * 3 / 4;
    if (d2 <= th && d2 < bestd) { bestd=d2; best=z; }
  }
  if (best != 255) {
    zones_[best].cx = sx; zones_[best].cy = sy;
    zones_[best].r  = r;
    zones_[best].active = true;
    zones_[best].stamp = millis();
    return;
  }

  if (zoneCount_ < MAX_ZONES) {
    zones_[zoneCount_] = {sx, sy, r, millis(), true};
    ++zoneCount_;
  } else {
    // evict oldest
    uint8_t oldest = 0;
    uint32_t ts = zones_[0].stamp;
    for (uint8_t z=1; z<MAX_ZONES; ++z) if (zones_[z].stamp < ts) { ts = zones_[z].stamp; oldest = z; }
    zones_[oldest] = {sx, sy, r, millis(), true};
  }
}

// integer-only collision resolve (no sqrt/float)
bool Experience_Sand::resolveBlocking(uint16_t &nx, uint16_t &ny, int16_t &vxQ, int16_t &vyQ) {
  bool hit = false;
  for (uint8_t z=0; z<zoneCount_; ++z) {
    const Zone &Z = zones_[z];
    if (!Z.r) continue;

    int32_t dx = (int32_t)nx - (int32_t)Z.cx;
    int32_t dy = (int32_t)ny - (int32_t)Z.cy;
    int32_t d2 = dx*dx + dy*dy;
    int32_t r2 = (int32_t)Z.r * (int32_t)Z.r;

    if (d2 < r2) {
      hit = true;
      if (dx==0 && dy==0) dx = 1;
      // push to boundary along dominant axis
      if (abs(dx) >= abs(dy)) nx = (dx > 0) ? (uint16_t)(Z.cx + Z.r) : (uint16_t)(Z.cx - Z.r);
      else                    ny = (dy > 0) ? (uint16_t)(Z.cy + Z.r) : (uint16_t)(Z.cy - Z.r);

      // dampen velocity (simulate sliding)
      vxQ = (int16_t)(vxQ * 3 / 4);
      vyQ = (int16_t)(vyQ * 3 / 4);
    }
  }
  return hit;
}
