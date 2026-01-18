/*
  TOF Sensor Gesture Detection
  (header comment unchanged)
*/

#include "TOF.h"

extern Video video;

// How long we keep reporting the last known finger position after tracking drops
static constexpr unsigned long FINGER_HOLD_MS = 250UL;

// A small denom threshold so we don’t treat tiny weights as “real”
static constexpr float CENTROID_DENOM_EPS = 1.0f;

// Helpers (file-local)
static inline int16_t clamp16(int16_t v, int16_t lo, int16_t hi)
{
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

TOF::TOF() {}

void TOF::begin()
{
  if (!tof.begin()) {
    Serial.println("TOF failed");
    video.stopOnError("TOF failed", "to start", " ", " ", " ");
  }

  // Configure sensor
  tof.setRangingFrequency(FRAME_RATE);
  tof.setResolution(64);

  if (!tof.startRanging())
  {
    Serial.println("TOF failed to start ranging. Stopping");
    video.stopOnError("TOF failed", "to start", "ranging", " ", " ");
  }

  lastRead   = millis();
  captTime   = 0;        // don't block gestures immediately after boot
  lastValid  = millis();
  circCnt    = 0;
  sleepStart = millis();

  tipPos  = 0;
  tipDist = 0;

  // Old min/max scaling no longer used for tip mapping (kept for compatibility)
  tipMin = 10;
  tipMax = 0;

  isRunning = false;

  pendingDirection = false;
  direction        = GESTURE_NONE;
  directionWay     = " ";

  for (int i = 0; i < WINDOW; i++) deltas[i] = 0;
  lastCentroid = 0;
  wi = 0;

  // finger tracking presence state
  fingerLastSeen = 0;
  fingerPresent  = false;

  // NEW: gesture gate state
  gateState       = GestureGateState::Armed;
  cooldownUntilMs = 0;

  mymessage  = "";
  mymessage2 = "";
}

void TOF::resetGesture() {
  pendingDirection = false;
  directionWay     = " ";
  direction        = GESTURE_NONE;
}

// Dump all WINDOW rotated[] frames (oldest→newest)
void TOF::prettyPrintAllRotated()
{
  Serial.println("---- Last 5 rotated frames (mm) ----");
  for (int f = 0; f < WINDOW; f++) {
    int idx = (wi + f) % WINDOW;
    Serial.print("Frame "); Serial.print( centroidFrames[idx] ); Serial.println(f);
    for (int r = 0; r < 8; r++) {
      for (int c = 0; c < 8; c++) {
        Serial.print(rotatedFrames[idx][r*8 + c]);
        Serial.print('\t');
      }
      Serial.println();
    }
    Serial.println();
  }
}

// For finger tip location analysis - no longer used
int TOF::mapFloatTo0_7(float input, float inMin, float inMax)
{
  if (inMax <= inMin) return 0;

  float norm = (input - inMin) / (inMax - inMin);
  norm = std::max(0.0f, std::min(1.0f, norm));
  int result = static_cast<int>(norm * 7.0f + 0.5f);
  return result;
}

int TOF::getGesture()
{
  int prevd = direction;
  direction = GESTURE_NONE;
  return prevd;
}

void TOF::setStatus(bool running) {
  isRunning = running;
  if (isRunning) {
    tof.startRanging();
  } else {
    tof.stopRanging();
  }
}

bool TOF::getStatus() {
  return isRunning;
}

int TOF::getFingerPos() {
  return tipPos;
}

float TOF::getFingerDist() {
  return tipDist;
}

String TOF::getRecentMessage()
{
  String myMefa = mymessage;
  mymessage = "";
  return myMefa;
}

String TOF::getRecentMessage2()
{
  String myMefa = mymessage2;
  mymessage2 = "";
  return myMefa;
}

/*
  Phase 1: Finger tracking
*/

static bool computeFingerTracking(
  const int16_t rotated[64],
  float &outCentroidX,
  int &outTipPos,
  float &outTipDist
)
{
  float num = 0.0f;
  float denom = 0.0f;

  // centroid over X only, weighted by closeness
  for (int i = 0; i < 64; i++) {
    int16_t d = rotated[i];
    if (d >= MIN_DIST_MM && d <= MAX_DIST_MM) {
      float w = float(MAX_DIST_MM - d);  // closer => higher weight
      num   += w * float(i % 8);
      denom += w;
    }
  }

  if (denom < CENTROID_DENOM_EPS) {
    return false;
  }

  float cx = num / denom; // ~0..7
  outCentroidX = cx;

  int col = (int)lroundf(cx);
  if (col < 0) col = 0;
  if (col > 7) col = 7;
  outTipPos = col;

  int16_t best = MAX_DIST_MM;
  for (int r = 0; r < 8; r++) {
    int idx = r*8 + col;
    int16_t d = rotated[idx];
    if (d >= MIN_DIST_MM && d <= MAX_DIST_MM) {
      if (d < best) best = d;
    }
  }
  outTipDist = float(best);

  return true;
}

/*
  Phase 2: Gesture detection
*/

void TOF::loop()
{
  if (!isRunning) return;

  uint32_t now = millis();

  // Frame rate pacing
  if (now - lastRead < FRAME_INTERVAL_MS) return;
  lastRead = now;

  VL53L5CX_ResultsData rd;
  if (!tof.getRangingData(&rd)) return;

  // Build tmp[] and count valid pixels and sleep pixels
  int16_t tmp[64];
  int validCnt = 0;
  int sleepCnt = 0;

  for (int i = 0; i < 64; i++) {
    int16_t d = rd.distance_mm[i];

    if (d >= SLEEP_MIN_DISTANCE && d <= SLEEP_MAX_DISTANCE) sleepCnt++;

    if (d >= MIN_DIST_MM && d <= MAX_DIST_MM) {
      tmp[i] = d;
      validCnt++;
    } else {
      tmp[i] = MAX_DIST_MM;
    }
  }

  // ——— Sleep detection (allowed even during cooldown) ———
  if (now - sleepStart > SLEEP_HOLD_MS)
  {
    sleepStart = now;

    if (sleepCnt >= SLEEP_COVERAGE_COUNT)
    {
      resetGesture();
      direction = GESTURE_SLEEP;
      directionWay = "→ Sleep";
      mymessage = directionWay;
      mymessage2 = String(sleepCnt);

      // Enter cooldown
      captTime = now;
      gateState = GestureGateState::Cooldown;
      cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

      return;
    }
  }

  // Rotate + flip into rotated[]
  int16_t rotated[64];
  for (int r = 0; r < 8; r++)
    for (int c = 0; c < 8; c++)
      rotated[c*8 + r] = tmp[r*8 + c];

  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 8; c++) {
      int16_t t = rotated[r*8 + c];
      rotated[r*8 + c] = rotated[(7-r)*8 + c];
      rotated[(7-r)*8 + c] = t;
    }

  // ---- PHASE 1: Finger tracking (ALWAYS, even during cooldown) ----
  float centroidX = 3.5f;
  int   newTipPos = tipPos;
  float newTipDist = tipDist;

  bool fingerOk = computeFingerTracking(rotated, centroidX, newTipPos, newTipDist);

  if (fingerOk) {
    tipPos = newTipPos;
    tipDist = newTipDist;
    fingerLastSeen = now;
    fingerPresent = true;
  } else {
    // If tracking drops, keep last tipPos briefly, then “neutral”
    if (fingerPresent && (now - fingerLastSeen) > FINGER_HOLD_MS) {
      fingerPresent = false;
      tipPos = 0;          // match your existing semantics (0 means none)
      tipDist = MAX_DIST_MM;
    }
  }

  // ---- NEW: Gesture gate (cooldown + require finger gone to re-arm) ----
  if (gateState == GestureGateState::Cooldown)
  {
    // Only re-arm when:
    //  1) cooldown time elapsed, AND
    //  2) finger is no longer present (hand moved away)
    if (now >= cooldownUntilMs && !fingerPresent) {
      gateState = GestureGateState::Armed;
    } else {
      // We still updated finger tracking above; just suppress gesture detection
      return;
    }
  }

  // ---- PHASE 2: Gesture detection (ONLY when enough pixels) ----
  if (validCnt < VALID_PIXEL_COUNT) {
    // Not enough signal to form gestures, but finger tracking already updated.
    return;
  }

  // Copy this frame into our circular buffer (for debugging/analysis)
  memcpy(rotatedFrames[wi], rotated, sizeof(rotated));
  centroidFrames[wi] = centroidX;

  // compute delta and store (gesture domain)
  float delta = centroidX - lastCentroid;
  sumDelta    = sumDelta - deltas[wi] + delta;
  deltas[wi]  = delta;
  lastCentroid = centroidX;

  // Ignore small movements; count toward circular gesture
  if (fabs(delta) < 0.08)
  {
    circCnt++;
    if (circCnt > CIRCULAR_MAX)
    {
      direction = GESTURE_CIRCULAR;
      directionWay = cirmesg;

      // Enter cooldown
      captTime = now;
      gateState = GestureGateState::Cooldown;
      cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

      circCnt = 0;
      return;
    }
    return;
  }

  // If too long since lastValid, reset window
  if (now - lastValid > MAX_SCANTIME)
  {
    lastValid = now;
    wi = 0;
    for (int i = 0; i < WINDOW; i++) deltas[i] = 0;
  }

  // Analysis: negative delta = moving right-to-left, positive = left-to-right
  int cposcon = 0, cnegcon = 0, cpos = 0, cneg = 0;
  for (int i = 0; i < WINDOW; i++) {
    int idx = (wi + i) % WINDOW;
    float d = deltas[idx];
    if (d > 0) cpos++;
    if (d < 0) cneg++;
    float dn = deltas[(idx + 1) % WINDOW];
    if (d > 0 && dn > 0) cposcon++;
    if (d < 0 && dn < 0) cnegcon++;
  }

  // advance write pointer
  wi = (wi + 1) % WINDOW;
  lastValid = now;

  if (cnegcon == 0 && cposcon == 0) {
    return;
  }

  if (cneg == 2 && cnegcon == 1 && cpos == 1 && cposcon == 0) {
    return;
  }

  if (cneg > 0 && cnegcon == 0 && cpos < 3 && cposcon < 2)
  {
    direction = GESTURE_LEFT_RIGHT;
    directionWay = lrmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  if (cneg > 1 && cnegcon > 1 && cpos > 0 && cposcon == 0)
  {
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  if (cneg > 0 && cneg < 3 && cnegcon < 2 && cpos == 0 && cposcon == 0)
  {
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  if (cneg > 0 && cpos > 0 && cneg > cpos && cnegcon > 0 && cposcon > 1)
  {
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  if (cneg > 0 && cpos > 0 && cneg < cpos && cnegcon > 1 && cposcon > 1)
  {
    direction = GESTURE_LEFT_RIGHT;
    directionWay = lrmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  if (cneg > 0 && cpos > 0 && cneg < cpos && cnegcon > 0 && cposcon > 1)
  {
    direction = GESTURE_LEFT_RIGHT;
    directionWay = lrmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  if (cneg > 1 && cnegcon > 0 && cpos > 0 && cposcon == 0)
  {
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }

  // fallthrough: count toward circular gesture
  circCnt++;
  if (circCnt > CIRCULAR_MAX)
  {
    direction = GESTURE_CIRCULAR;
    directionWay = cirmesg;

    // Enter cooldown
    captTime = now;
    gateState = GestureGateState::Cooldown;
    cooldownUntilMs = now + (uint32_t)GESTURE_WAIT;

    circCnt = 0;
    return;
  }
}