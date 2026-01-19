/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for interacting with the video display.

Static JPEG images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

Some classes use the framebuffer capability in Arduino_GFX. Tutorial is at
https://github.com/moononournation/Arduino_GFX/wiki/Canvas-Class
Note: ran out of memory, so no frame buffering right now.

Static JPEG images must be 240x240 and use JPEG baseline encoding. I use ffmpeg to convert a
JPEG created in Pixelmator on Mac OS to baseline encoding using:
ffmpeg -i cat1_parallax.jpg -q:v 2 -vf "format=yuvj420p" cat1_parallax_baseline.jpg

*/

#include "Video.h"

#include "Hardware.h"
extern Hardware hardware;

// ------------------------------------------------------------
// Spiral text demo (tangent-only, shrinking, greeking)
// ------------------------------------------------------------
//
// This stays entirely buffer-free (plots glyph pixels directly).
// Toggle with SPIRAL_TEXT_DEMO.
//
#ifndef SPIRAL_TEXT_DEMO
#define SPIRAL_TEXT_DEMO 1
#endif

#if SPIRAL_TEXT_DEMO

#include <math.h>

// These fonts are provided by your project (uploaded in this thread).
// Minya16pt7b and ScienceFair14pt7b are already included via Video.h.
#include "SomeTimeLater20pt7b.h"
#include "Minya_Nouvelle_Rg30pt7b.h"

// gfx is defined later in this file; forward declare so the demo helpers can use it.
extern Arduino_GFX *gfx;

enum class SpiralMode : uint8_t { Outward = 0, Inward = 1 };
enum class TurnDir : int8_t { CCW = +1, CW = -1 };

static inline float deg2rad_(float d) { return d * 0.017453292519943295769f; }
static inline int16_t iround_(float x) { return (int16_t)lroundf(x); }

// Deterministic hash for dithering/jitter (no RNG state, no heap)
static inline uint32_t hash32_(uint32_t x)
{
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}

static void drawGlyphTangentScaled_(
  Arduino_GFX* g,
  const GFXfont* font,
  const GFXglyph* glyph,
  int16_t bx,
  int16_t by,
  float tangentAngleRad,
  float scale,
  uint16_t color,
  bool greek,
  float greekDotFactor,
  uint32_t seed,
  bool allowDither                 // <-- NEW: can disable pixel-drop dithering
)
{
  if (!g || !font || !glyph) return;

  // Greeking: show a dot sized relative to the (scaled) character height,
  // with slight, deterministic jitter so it reads as "greeked text" rather
  // than a perfectly uniform bead string.
  if (greek)
  {
    int16_t yAdv = (int16_t)pgm_read_byte(&font->yAdvance);
    float dotR = greekDotFactor * (float)yAdv * scale;
    int16_t r = (int16_t)lroundf(dotR);
    if (r < 1) r = 1;

    // Jitter along the tangent and normal (deterministic)
    uint32_t h = hash32_(seed ^ 0xA5A5u);
    float jt = (float)((int)((h >> 0) & 7) - 3) * 0.35f * scale; // small
    float jn = (float)((int)((h >> 3) & 7) - 3) * 0.25f * scale;

    float ct = cosf(tangentAngleRad);
    float st = sinf(tangentAngleRad);
    // tangent unit (ct, st), normal is (-st, ct)
    int16_t dx = iround_(jt * ct + jn * (-st));
    int16_t dy = iround_(jt * st + jn * (ct));

    // Pull dots slightly toward the inside of the stroke (aesthetic)
    float inset = 0.18f * (float)yAdv * scale;
    dx += iround_((-st) * (-inset));
    dy += iround_((ct) * (-inset));

    g->fillCircle(bx + dx, by + dy, r, color);
    return;
  }

  const uint8_t* bitmap = font->bitmap;

  uint16_t bo = (uint16_t)pgm_read_word(&glyph->bitmapOffset);
  uint8_t  w  = (uint8_t) pgm_read_byte(&glyph->width);
  uint8_t  h  = (uint8_t) pgm_read_byte(&glyph->height);
  int8_t   xo = (int8_t)  pgm_read_byte(&glyph->xOffset);
  int8_t   yo = (int8_t)  pgm_read_byte(&glyph->yOffset);

  float ct = cosf(tangentAngleRad);
  float st = sinf(tangentAngleRad);

  // Smoother minification: deterministic dithering to drop pixels as scale decreases.
  // Option #1: allow turning dithering off entirely for solid strokes (Minya 30pt).
  float keepProb = 1.0f;

  if (allowDither && (scale < 1.0f))
  {
    keepProb = scale * scale; // bias toward keeping more pixels longer
    if (keepProb < 0.12f) keepProb = 0.12f; // avoid vanishing too early
  }

  // If dithering is disabled, keepThresh=1023 means "never drop"
  const uint16_t keepThresh = allowDither
    ? (uint16_t)lroundf(keepProb * 1023.0f)
    : 1023;

  uint16_t bitIndex = 0;
  for (uint8_t yy = 0; yy < h; yy++)
  {
    for (uint8_t xx = 0; xx < w; xx++)
    {
      // advance bitIndex no matter what (bitmap is tightly packed)
      uint16_t byteIndex = bo + (bitIndex >> 3);
      uint8_t  bitMask   = 0x80 >> (bitIndex & 7);
      uint8_t  b = pgm_read_byte(bitmap + byteIndex);

      bool on = (b & bitMask) != 0;
      bitIndex++;

      if (!on) continue;

      // Deterministic dither: decide whether to plot this pixel.
      if (keepThresh < 1023)
      {
        // Mix pixel coords + seed so the pattern is stable per-character.
        uint32_t mix = seed ^ ((uint32_t)xx * 73856093u) ^ ((uint32_t)yy * 19349663u);
        uint16_t r01 = (uint16_t)(hash32_(mix) & 1023u);
        if (r01 > keepThresh) continue;
      }

      // Glyph pixel local coords => baseline-relative offsets using xOffset/yOffset.
      float lx = (float)(xo + (int)xx) * scale;
      float ly = (float)(yo + (int)yy) * scale;

      float rx = lx * ct - ly * st;
      float ry = lx * st + ly * ct;

      // -----------------------------
      // Option #3: "Coverage" splat AA
      // -----------------------------
      // We can't do true grayscale AA with 1-bit fonts, but we can fill holes
      // by splatting into neighbor pixels based on subpixel position.
      //
      // This makes small text much less "holey" than fixed micro-bold.
      float xf = (float)bx + rx;
      float yf = (float)by + ry;

      int16_t x0 = (int16_t)floorf(xf);
      int16_t y0 = (int16_t)floorf(yf);

      float fx = xf - (float)x0;   // 0..1
      float fy = yf - (float)y0;   // 0..1

      // Always draw the base pixel
      g->drawPixel(x0, y0, color);

      // How much "extra coverage" we want: none at full size, more when smaller.
      // Tunable: 0.0 (no AA) .. about 0.9 (pretty filled).
      float aa = 0.0f;
      if (scale < 1.0f) {
        aa = 1.0f - scale;           // smaller scale => more AA fill
        if (aa > 0.85f) aa = 0.85f;  // clamp
      }

      // Deterministic dither decision helper (stable, no RNG state)
      auto ditherHit = [&](uint32_t tag, float prob) -> bool {
        if (prob <= 0.0f) return false;
        if (prob >= 1.0f) return true;
        uint32_t h = hash32_(seed ^ tag);
        // 0..1023
        uint16_t r01 = (uint16_t)(h & 1023u);
        uint16_t thr = (uint16_t)lroundf(prob * 1023.0f);
        return r01 <= thr;
      };

      // Bilinear-inspired neighbor "coverage" probabilities.
      // fx/fy decide *which* neighbors matter; aa decides *how much*.
      // Right neighbor
      if (ditherHit(0x1111u ^ ((uint32_t)x0 << 16) ^ (uint32_t)y0, fx * aa)) {
        g->drawPixel(x0 + 1, y0, color);
      }
      // Down neighbor
      if (ditherHit(0x2222u ^ ((uint32_t)x0 << 16) ^ (uint32_t)y0, fy * aa)) {
        g->drawPixel(x0, y0 + 1, color);
      }
      // Diagonal neighbor
      if (ditherHit(0x3333u ^ ((uint32_t)x0 << 16) ^ (uint32_t)y0, (fx * fy) * aa)) {
        g->drawPixel(x0 + 1, y0 + 1, color);
      }

      // (Optional) a tiny extra fill at very small sizes to prevent sparkle-holes
      if (scale < 0.45f) {
        if (ditherHit(0x4444u ^ ((uint32_t)x0 << 16) ^ (uint32_t)y0, 0.20f)) {
          g->drawPixel(x0 - 1, y0, color);
        }
      }
    }
  }
}

static void drawSpiralText_(
  Arduino_GFX* g,
  const GFXfont* font,
  const char* text,
  int16_t cx,
  int16_t cy,
  float r0Px,
  float kPxPerRad,
  float theta0Rad,
  SpiralMode mode,
  TurnDir turnDir,
  bool centerAlign,
  float scaleStart,
  float shrinkPerRad,
  float minReadableScale,
  float greekDotFactor,
  int16_t letterSpacingPx,
  uint16_t color
)
{
  if (!g || !font || !text) return;

  // --- Layout polish: center the string around theta0Rad (best-effort) ---
  // This pre-walk uses the *same* stepping/shrinking logic as drawing, but
  // without plotting pixels. Because the spiral equation is based on
  // (theta - theta0), shifting theta0 merely rotates the whole layout.
  if (centerAlign)
  {
    uint8_t first = pgm_read_byte(&font->first);
    uint8_t last  = pgm_read_byte(&font->last);
    const float dir = (mode == SpiralMode::Outward) ? +1.0f : -1.0f;

    float theta = theta0Rad;
    float scale = scaleStart;
    for (const char* p = text; *p; ++p)
    {
      uint8_t c = (uint8_t)(*p);
      if (c < first || c > last) c = '?';
      const GFXglyph* glyph = font->glyph + (c - first);
      int8_t xAdv = (int8_t)pgm_read_byte(&glyph->xAdvance);

      float r = r0Px + dir * kPxPerRad * (theta - theta0Rad);
      if (r < 2.0f) r = 2.0f;

      float advScaled = ((float)xAdv + (float)letterSpacingPx) * scale;
      float dTheta = (r > 1.0f) ? (advScaled / r) : 0.0f;
      theta += (float)((int)turnDir) * dTheta;
      scale *= expf(-shrinkPerRad * fabsf(dTheta));
      if (scale < 0.03f) break;
    }

    const float span = (theta - theta0Rad);
    theta0Rad -= (float)((int)turnDir) * (span * 0.5f);
  }

  uint8_t first = pgm_read_byte(&font->first);
  uint8_t last  = pgm_read_byte(&font->last);

  const float dir = (mode == SpiralMode::Outward) ? +1.0f : -1.0f;

  float theta = theta0Rad;
  float scale = scaleStart;

  uint32_t idx = 0;
  for (const char* p = text; *p; ++p, ++idx)
  {
    uint8_t c = (uint8_t)(*p);
    if (c < first || c > last) c = '?';
    const GFXglyph* glyph = font->glyph + (c - first);

    int8_t xAdv = (int8_t)pgm_read_byte(&glyph->xAdvance);

    // Current radius
    float dThetaFromStart = (theta - theta0Rad);
    float r = r0Px + dir * kPxPerRad * dThetaFromStart;
    if (r < 2.0f) r = 2.0f;

    // Baseline point on spiral
    float cth = cosf(theta);
    float sth = sinf(theta);
    int16_t bx = (int16_t)(cx + iround_(r * cth));
    int16_t by = (int16_t)(cy + iround_(r * sth));

    // Tangent direction for Archimedean spiral:
    // r = r0 + dir*k*(theta-theta0), dr/dtheta = dir*k
    // tangent angle psi = theta + atan2(r, dr/dtheta)
    float drdtheta = dir * kPxPerRad;
    float psi = theta + atan2f(r, drdtheta);

    const bool greek = (scale < minReadableScale);

    // Seed: stable per character (used for dithering + greeking jitter)
    uint32_t seed = ((uint32_t)c) | (idx << 8) | ((uint32_t)pgm_read_word(&glyph->bitmapOffset) << 16);

    // Option #1: disable dithering for the big 30pt Minya font (solid strokes).
    const bool allowDither = (font != &Minya_Nouvelle_Rg30pt7b);

    drawGlyphTangentScaled_(
      g,
      font,
      glyph,
      bx,
      by,
      psi,
      scale,
      color,
      greek,
      greekDotFactor,
      seed,
      allowDither
    );

    // Advance along spiral using scaled xAdvance
    float advScaled = ((float)xAdv + (float)letterSpacingPx) * scale;
    float dTheta = (r > 1.0f) ? (advScaled / r) : 0.0f;
    theta += (float)((int)turnDir) * dTheta;

    // Shrink based on absolute angular travel (works the same inward/outward)
    scale *= expf(-shrinkPerRad * fabsf(dTheta));

    if (scale < 0.03f) break;
  }
}

static void spiralTextDemo_()
{
  // Ensure backlight is on for the demo.
  digitalWrite(Display_SPI_BK, LOW);

  gfx->fillScreen(BLACK);

  const int16_t cx = 120;
  const int16_t cy = 120;

  // Spiral geometry
  const float r0 = 92.0f;
  const float k  = 2.6f;              // ~16px per turn: 2πk ≈ 16.3

  // Shrink & greek
  const float scaleStart = 1.0f;
  const float shrinkPerRad = 1.25f;
  const float minReadableScale = 0.40f;
  const float greekDotFactor = 0.10f;
  const int16_t letterSpacing = 1;

  // Outward spiral (YELLOW) using Minya
  drawSpiralText_(
    gfx,
    &Minya16pt7b,
    "3 Where Can I GET me some of those? The definition of an outward spiral",
    cx,
    cy,
    r0,
    k,
    deg2rad_(-90.0f),
    SpiralMode::Outward,
    TurnDir::CCW,
    true, // centerAlign
    scaleStart,
    shrinkPerRad,
    minReadableScale,
    greekDotFactor,
    letterSpacing,
    COLOR_TEXT_YELLOW
  );

  // Inward spiral (BLUE) using ScienceFair (reads nicely as it shrinks)
  drawSpiralText_(
    gfx,
    &ScienceFair14pt7b,
    "2 of those FOXES look tasty to eat. Fox who jumped over the dog lives here",
    cx,
    cy,
    r0,
    k,
    deg2rad_(90.0f),
    SpiralMode::Inward,
    TurnDir::CW,
    true, // centerAlign
    scaleStart,
    shrinkPerRad,
    minReadableScale,
    greekDotFactor,
    letterSpacing,
    BLUE
  );

  // Third spiral: big Minya (30pt) with later greeking (you tuned these).
  drawSpiralText_(
    gfx,
    &Minya_Nouvelle_Rg30pt7b,
    "1 The sparkly fade for Minya when it lives on and on and on. When and where will it end. I really don't know.",
    cx,
    cy,
    70.0f,            // r0 (smaller so the big font starts nearer center)
    2.9f,             // k (slightly wider turns for the big font)
    deg2rad_(180.0f), // start at left
    SpiralMode::Outward,
    TurnDir::CCW,
    true,             // centerAlign
    1.0f,             // scaleStart
    0.30f,            // shrinkPerRad (shrinks slower than 1.70f)
    0.20f,            // minReadableScale (greeks later than 0.62f)
    0.10f,            // greekDotFactor
    1,                // letterSpacing
    COLOR_TEXT_YELLOW
  );

  // Give you time to see it.
  delay(40000);

  // Clear back to black so the rest of the system can draw normally.
  gfx->fillScreen(BLACK);
}

#endif // SPIRAL_TEXT_DEMO

static Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, Display_SPI_RST, 1 /* rotation */, true /* IPS */);

Video::Video() {}

void Video::begin()
{
  #ifdef GFX_EXTRA_PRE_INIT
    GFX_EXTRA_PRE_INIT();
  #endif

  if ( ! gfx->begin() )
  {
    Serial.println(F("gfx->begin() failed. Stopping."));
    while(1);
  }

  gfx->fillScreen( BLACK );
  delay(300);

  gfx->fillCircle( 120, 120, 5, COLOR_PANTONE_310 );
  delay(300);
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillCircle( 140, 120, 5, COLOR_PANTONE_102 );
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillCircle( 120, 140, 5, COLOR_PANTONE_102 );
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillCircle( 100, 120, 5, COLOR_PANTONE_102 );
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);
  gfx->fillTriangle( 115, 102, 120, 90, 125, 102, COLOR_PANTONE_151);
  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
  delay(300);

  videoStatus = false;   // idle
  vidtimer = millis();

  curr_ms = millis();
  videoStartTime = millis();

  Serial.println( "Video started" );

  #if SPIRAL_TEXT_DEMO
    spiralTextDemo_();
  #endif
}

void Video::addReadTime( unsigned long rtime )
{
  totalReadVideo += rtime;
}

/* Show error on display, then halt */

void Video::stopOnError( String msg1, String msg2, String msg3, String msg4, String msg5 )
{
  gfx->fillScreen( COLOR_BACKGROUND );

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on

  String errmsg = F("Video stopOnError ");
  errmsg += msg1;
  errmsg += F(", ");
  errmsg += msg2;
  errmsg += F(", ");
  errmsg += msg3;
  errmsg += F(", ");
  errmsg += msg4;
  errmsg += F(", ");
  errmsg += msg5;
  logger.error( errmsg );

  ringtimer = millis();

  gfx->fillScreen( COLOR_PANTONE_577 );
  gfx -> fillRect( 40, 40, 160, 160, COLOR_PANTONE_662 );
  gfx -> drawRect( 39, 39, 162, 162, COLOR_PANTONE_102 );
  gfx -> drawRect( 40, 40, 160, 160, COLOR_PANTONE_151 );

  gfx->setFont(&ScienceFair14pt7b);
  gfx->setTextColor( COLOR_PANTONE_102 );
  gfx->setCursor( leftmargin, topmargin - 5 );
  gfx->println(F("OH BOTHER"));

  gfx->setCursor( leftmargin, topmargin + ( 1 * linespacing ) );
  gfx->setFont(&ScienceFair14pt7b);
  gfx->setTextColor( COLOR_PANTONE_310 );
  gfx->println( msg1 );

  gfx->setCursor( leftmargin, topmargin + ( 2 * linespacing ) );
  gfx->println( msg2 );

  gfx->setCursor( leftmargin, topmargin + ( 3 * linespacing ) );
  gfx->println( msg3 );

  gfx->setCursor( leftmargin, topmargin + ( 4 * linespacing ) );
  gfx->println( msg4 );

  gfx->setCursor( leftmargin, topmargin + ( 5 * linespacing ) );
  gfx->println( msg5 );
  
  while (1)
  {
    delay(500);
  }  
}

// Returns milliseconds since the video started playing

unsigned long Video::getVideoTime()
{
  return millis() - videoStartTime;
}

void Video::displayCentered( String msg, int yq )
{
  gfx->setFont( &Minya16pt7b );
  gfx->setTextSize(1);
  gfx->getTextBounds( msg.c_str(), 0, 0, &xp, &yp, &wp, &hp);
  gfx->setCursor( (gfx->width() - wp) / 2, 85 + ( hp * yq ) );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->println( msg );
}

void Video::displayTextMessage( String msg, String msg2, String msg3, String msg4 )
{
  gfx->begin();
  gfx->fillScreen( COLOR_PANTONE_662 );
  displayCentered( msg, 0 );
  displayCentered( msg2, 1 );
  displayCentered( msg3, 2 );
  displayCentered( msg4, 3 );
}

void Video::addCRLF(String &s, size_t lineLen) 
{
  size_t pos = lineLen;
  while (pos < s.length()) {
    // take everything up to pos, add "\r\n", then the rest
    s = s.substring(0, pos)
      + "\r\n   "
      + s.substring(pos);
    pos += lineLen + 2;   // skip over the chunk we just processed + the CRLF
  }
}

// Paint debug info over the display

void Video::paintText( String mef )
{
  gfx->setFont(nullptr);           // NULL = default system font (5×7)
  gfx->setTextSize(2);             // leave at 1 for true 5×7
  gfx->setCursor( 30, 30 );
  gfx->setTextColor( WHITE, BLUE );
  gfx->println( mef );
}

void Video::resetStats()
{
  totalFrames = 0;
  totalReadVideo = 0;
  totalDecodeVideo = 0;
  totalShowVideo = 0; 
  startMs = millis();
}

bool Video::getStatus()
{
  return videoStatus;
}

/* Play a .mjpeg file to the display */

void Video::startVideo( String vname )
{
  String mef = F("/");
  mef += NAND_BASE_DIR;
  mef += F("/");
  mef += vname;
  mef += F("/");
  mef += vname;
  mef += videoname_end;

  String msg = F("startVideo ");
  msg += mef;
  logger.info( msg );
  
  mjpegFile = SD.open( mef );
  if ( ! mjpegFile )
  {
    videoStatus = false;
    String msg = F("startVideo failed to open ");
    msg += mef;
    logger.error( msg );
    return;
  }

  if ( mjpegrunner.start( &mjpegFile ) )
  {
    videoStatus = true;
  }
  else
  {
    Serial.println( F( "MjpegRunner did not start") );
    videoStatus = false;
    return;
  }

  videoStartTime = millis();

  digitalWrite(Display_SPI_BK, LOW);  // Turn display backlight on
}

void Video::stopVideo()
{
  if (mjpegFile && mjpegFile.available()) 
  {
    mjpegFile.close();
  }
  setPaused( false );
  videoStatus = false;
}

void Video::setPaused( bool p )
{
  paused = p;
}

void Video::loop()
{
  if ( ( ! videoStatus ) || paused ) return;

  if ( (millis() - vidtimer ) > 50 ) 
  {
    if ( mjpegFile.available() )
    {
      vidtimer = millis();

      unsigned long dtime = millis();

      if ( ! mjpegrunner.readMjpegBuf() )
      {
        stopVideo();
        return;
      }

      totalFrames++;

      mjpegrunner.drawJpg();

      totalShowVideo += millis() - dtime;
    }
    else
    {
      stopVideo();
    }
  }
}