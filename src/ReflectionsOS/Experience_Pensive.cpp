#include "Experience_Pensive.h"

constexpr uint16_t Experience_Pensive::kRamp[5];

Experience_Pensive::Experience_Pensive() {}

void Experience_Pensive::init()
{
  setExperienceName(pensiveName);

  setSetupComplete(false);
  setRunComplete(false);
  setTeardownComplete(false);

  setStopped(false);
  setIdle(false);
  
  tearflag = true;

  startMs_ = millis();
  lastMs_ = startMs_;
}

void Experience_Pensive::setup()
{
  gfx->fillScreen(kBg);

  wellX_ = targetWellX_ = kCX;
  wellY_ = targetWellY_ = kCY;

  tiltX_ = tiltY_ = tiltMag_ = 0.0f;

  seedParticles_();

  setSetupComplete(true);
}

void Experience_Pensive::run()
{
  if (millis() - startMs_ > kDurationMs)
  {
    setRunComplete(true);
    return;
  }

  uint32_t now = millis();
  uint32_t dms = now - lastMs_;
  lastMs_ = now;

  // dt in "frames" (~60fps baseline), clamped
  float dt = clampf_(dms / 16.666f, 0.5f, 2.0f);

  readTilt_();
  updateWellFromTilt_();
  stepParticles_(dms, dt);
  render_();
}

void Experience_Pensive::teardown()
{
  // One-time teardown block (pattern used elsewhere in your codebase)
  if (tearflag)
  {
    tearflag = false;

    // Make absolutely sure ExperienceService won't keep us running
    setRunComplete(true);
    setStopped(true);
    setIdle(true);

    // Clear out Pensive trails so they don't "ghost" over WatchFaceMain
    gfx->fillScreen(0x0000);   // BLACK

    // Force a full watchface redraw next frame
    watchfacemain.setDrawItAll();

    // Optional but very effective: draw it immediately once
    // so the user sees an instant clean return.
    watchfacemain.loop();
  }

  setTeardownComplete(true);
}

// ------------------------------------------------------------
// Tilt + Well
// ------------------------------------------------------------

float Experience_Pensive::applyDeadZone_(float v, float dz)
{
  float av = fabsf(v);
  if (av < dz) return 0.0f;

  float sign = (v < 0) ? -1.0f : 1.0f;
  float out = (av - dz) / (1.0f - dz);
  return sign * clampf_(out, 0.0f, 1.0f);
}

void Experience_Pensive::readTilt_()
{
  // Your requested API
  float ax = accel.getXreading();
  float ay = accel.getYreading();

  // Normalize to approx [-1..+1] (your prior -6..+6 usage)
  float nx = clampf_(ax / 6.0f, -1.0f, 1.0f);
  float ny = clampf_(ay / 6.0f, -1.0f, 1.0f);

  // Dead-zone to prevent drift at rest
  nx = applyDeadZone_(nx, kTiltDeadZone);
  ny = applyDeadZone_(ny, kTiltDeadZone);

  // If motion feels reversed, flip an axis here:
  // nx = -nx;
  // ny = -ny;

  tiltX_ = nx;
  tiltY_ = ny;

  tiltMag_ = fastLen_(tiltX_, tiltY_);
  tiltMag_ = clampf_(tiltMag_, 0.0f, 1.0f);
}

void Experience_Pensive::updateWellFromTilt_()
{
  float ox = tiltX_ * kWellMaxOffset;
  float oy = tiltY_ * kWellMaxOffset;

  // Clamp offset within a circle
  float olen = fastLen_(ox, oy);
  if (olen > kWellMaxOffset)
  {
    float s = kWellMaxOffset / (olen + 0.0001f);
    ox *= s;
    oy *= s;
  }

  targetWellX_ = kCX + ox;
  targetWellY_ = kCY + oy;

  // Smooth well motion = "heavy liquid"
  wellX_ = wellX_ + kWellSmooth * (targetWellX_ - wellX_);
  wellY_ = wellY_ + kWellSmooth * (targetWellY_ - wellY_);
}

// ------------------------------------------------------------
// Particles
// ------------------------------------------------------------

float Experience_Pensive::rand01_(uint32_t &s)
{
  // xorshift32
  s ^= s << 13;
  s ^= s >> 17;
  s ^= s << 5;
  return (float)(s & 0x00FFFFFF) / (float)0x01000000;
}

uint16_t Experience_Pensive::lerp565_(uint16_t a, uint16_t b, float t)
{
  t = clampf_(t, 0.0f, 1.0f);

  int ar = (a >> 11) & 31;
  int ag = (a >> 5)  & 63;
  int ab = (a >> 0)  & 31;

  int br = (b >> 11) & 31;
  int bg = (b >> 5)  & 63;
  int bb = (b >> 0)  & 31;

  int rr = (int)(ar + (br - ar) * t);
  int rg = (int)(ag + (bg - ag) * t);
  int rb = (int)(ab + (bb - ab) * t);

  return (uint16_t)((rr << 11) | (rg << 5) | (rb));
}

uint16_t Experience_Pensive::ramp565_(float t01)
{
  t01 = clampf_(t01, 0.0f, 1.0f);

  // 5 stops => 4 segments
  const float seg = t01 * 4.0f;
  int i = (int)seg;               // 0..4
  if (i >= 4) return kRamp[4];

  float local = seg - (float)i;   // 0..1 within segment
  return lerp565_(kRamp[i], kRamp[i + 1], local);
}

void Experience_Pensive::seedParticles_()
{
  for (int i = 0; i < kMaxP; i++)
  {
    p_[i].seed = 0xA341316Cu ^ (i * 2654435761u) ^ millis();
    spawnOne_(i);
  }
}

void Experience_Pensive::spawnOne_(int i)
{
  P &p = p_[i];
  uint32_t &s = p.seed;

  float a = rand01_(s) * 6.2831853f;
  float r = kSpawnRadius + (rand01_(s) - 0.5f) * 10.0f;

  p.x = kCX + cosf(a) * r;
  p.y = kCY + sinf(a) * r;

  // initial tangential velocity
  float tx = -sinf(a);
  float ty =  cosf(a);
  float v  = 0.8f + rand01_(s) * 1.6f;

  p.vx = tx * v;
  p.vy = ty * v;

  // slight inward nudge
  p.vx += (kCX - p.x) * 0.002f;
  p.vy += (kCY - p.y) * 0.002f;

  p.mass  = 0.75f + rand01_(s) * 1.2f;
  p.alpha = 0.35f + rand01_(s) * 0.55f;

  // Lifetime: pick a TTL and reset age
  // (This is what drives the color ramp)
  p.ttlMs = 2500 + (uint32_t)(rand01_(s) * 4500.0f); // 2.5s .. 7.0s
  p.ageMs = (uint32_t)(rand01_(s) * 700.0f);         // small staggering

  // Initial color
  p.color = ramp565_((float)p.ageMs / (float)p.ttlMs);
}

void Experience_Pensive::stepParticles_(uint32_t dms, float dt)
{
  // Swirl strength grows with tilt magnitude
  float swirlStrength = kSwirlBase + tiltMag_ * kSwirlTiltGain;

  // Swirl direction changes with tilt (bias CW vs CCW)
  float swirlSign = clampf_(tiltX_ * kSwirlDirBias, -1.0f, 1.0f);
  if (tiltMag_ < 0.12f) swirlSign *= (tiltMag_ / 0.12f);

  float swirl = swirlStrength * swirlSign;

  // Gravity slightly increases with tilt -> feels like pouring
  float G = kBaseG * (1.0f + 0.25f * tiltMag_);

  for (int i = 0; i < kMaxP; i++)
  {
    P &p = p_[i];

    // lifetime + color ramp
    p.ageMs += dms;
    float life01 = (p.ttlMs > 0) ? ((float)p.ageMs / (float)p.ttlMs) : 1.0f;
    if (life01 >= 1.0f)
    {
      spawnOne_(i);
      continue;
    }
    p.color = ramp565_(life01);

    float dx = wellX_ - p.x;
    float dy = wellY_ - p.y;

    float r2  = dx*dx + dy*dy + kSoftening;
    float r   = sqrtf(r2);
    float inv = 1.0f / (r + 0.0001f);

    // inward pull
    float ax = (dx * inv) * (G / p.mass);
    float ay = (dy * inv) * (G / p.mass);

    // tangential spiral component
    float tx = -dy * inv;
    float ty =  dx * inv;

    ax += tx * swirl;
    ay += ty * swirl;

    // wobble = "liquid memory"
    uint32_t s = p.seed;
    float wob = (rand01_(s) - 0.5f) * 0.06f;
    p.seed = s;

    ax += wob * tx;
    ay += wob * ty;

    // integrate velocity
    p.vx += ax * dt;
    p.vy += ay * dt;

    // drag
    p.vx *= kDrag;
    p.vy *= kDrag;

    // cap speed
    float sp = fastLen_(p.vx, p.vy);
    if (sp > kMaxSpeed)
    {
      float ss = kMaxSpeed / (sp + 0.0001f);
      p.vx *= ss;
      p.vy *= ss;
    }

    // integrate position
    p.x += p.vx * dt;
    p.y += p.vy * dt;

    // recycle near well
    if (r < kKillRadius)
    {
      spawnOne_(i);
      continue;
    }

    // rare offscreen
    if (p.x < -10 || p.x > kW + 10 || p.y < -10 || p.y > kH + 10)
    {
      spawnOne_(i);
      continue;
    }
  }
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------

void Experience_Pensive::render_()
{
  // Dither-clear for trails
  uint32_t t = millis();
  for (int y = 0; y < kH; y += 2)
  {
    for (int x = (y & 2); x < kW; x += 4)
    {
      if (((x + y + (t >> 4)) & 7) == 0)
        gfx->drawPixel(x, y, kBg);
    }
  }

  // Particles
  for (int i = 0; i < kMaxP; i++)
  {
    P &p = p_[i];
    int x = (int)p.x;
    int y = (int)p.y;

    if ((unsigned)x >= kW || (unsigned)y >= kH) continue;

    gfx->fillCircle(x, y, kParticleSize, p.color);

    // sparkle
    if (((p.seed + millis()) & 127) == 0)
      gfx->drawPixel(x + 1, y, 0xFFFF);
  }

  // Draw vortex core last (on top) using /REFLECTIONS/Sand_small.png
  // Center the PNG on the well
  int16_t x0 = (int16_t)wellX_ - (kCoreW / 2);
  int16_t y0 = (int16_t)wellY_ - (kCoreH / 2);

  // This is the same path Experience_Sand uses.
  // "true" means embellish filename with /REFLECTIONS/ folder inside drawImageFromFile()
  watchfacemain.drawImageFromFile(kCorePng, true, x0, y0);
}