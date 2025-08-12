/*
  TOF Sensor Gesture Detection

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library

  Reflections board uses a Time Of Flight (TOF) VL53L5CX sensor to
  identify user gestures with their fingers and hand. Gestures control
  operating the Experiences.

  Note: I tried Lucas–Kanade optical‐flow and kept coming back to
  centroid differentials. I expect with more work Lucas-Kanade is more
  accurate, ran out of time to find out.

  Datasheet comes with this source code, see: vl53l5cx-2886943_.pdf
*/

#include "TOF.h"

extern Video video;

TOF::TOF() {}

void TOF::begin() 
{
  if (!tof.begin()) {
    Serial.println("TOF failed");
    video.stopOnError( "TOF failed", "to start", " ", " ", " " );
  }
 
  // Configure sensor
  tof.setRangingFrequency(FRAME_RATE);
  tof.setResolution(64);

  if (!tof.startRanging()) 
  {
    Serial.println("TOF failed to start ranging. Stopping");
    video.stopOnError( "TOF failed", "to start", "ranging", " ", " " );
  }

  lastRead = millis();
  captTime = millis();
  lastValid = millis();
  circCnt = 0;
  sleepStart  = millis();

  tipPos = 0;
  tipDist = 0;
  tipMin = 10;
  tipMax = 0;

  isRunning        = false;

  pendingDirection = false;
  direction        = GESTURE_NONE;
  directionWay     = " ";

  for ( int i = 0; i< WINDOW; i++ ) deltas[ i ] = 0;
  lastCentroid = 0;
  wi = 0;

}

void TOF::resetGesture() {
  pendingDirection = false;
  directionWay     = " ";
  direction        = GESTURE_NONE;
}

// ——— Dump all WINDOW rotated[] frames (oldest→newest) ——————————————
void TOF::prettyPrintAllRotated() {
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

// For finger tip location analysis
int TOF::mapFloatTo0_7(float input, float inMin, float inMax) 
{
    // Avoid division by zero
    if (inMax <= inMin) return 0;

    // 1) Normalize to [0,1]
    float norm = (input - inMin) / (inMax - inMin);

    // 2) Clamp between 0 and 1
    norm = std::max(0.0f, std::min(1.0f, norm));

    // 3) Scale to [0..7] and round
    //    – multiply by 7 gives [0..7]
    //    – +0.5 then truncate gives proper rounding
    int result = static_cast<int>(norm * 7.0f + 0.5f);

    return result;  // guaranteed 0 ≤ result ≤ 7
}

int TOF::getGesture() 
{
  int prevd = direction;
  direction = GESTURE_NONE;
  return prevd;
}

void TOF::setStatus(bool running) {
  isRunning = running;
  if ( isRunning )
  {
    tof.startRanging();
  }
  else
  {
    tof.stopRanging();
  }
}

bool TOF::getStatus() {
  return isRunning;
}

int TOF::getFingerPos() {
  return tipPos;
}

float TOF::getFingerDist(){
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

void TOF::loop() {
  if (!isRunning ) return;

  unsigned long now = millis();

  // Enforce lockout after a gesture is reported
  if (now - captTime < GESTURE_WAIT) return;

  if (now - lastRead < FRAME_INTERVAL_MS) return;
  lastRead = now;

  VL53L5CX_ResultsData rd;

  if (!tof.getRangingData(&rd)) return;

  // build a tmp[] and count valid pixels and sleep pixels
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

  // ——— Sleep detection ———
  if ( now - sleepStart > SLEEP_HOLD_MS )
  {
    sleepStart = now;

    /*
    // Debugging
    String mez = "TOF ";
    mez += sleepCnt;
    mez += "\n";
    for (int r = 0; r < 8; r++) 
    {
      for (int c = 0; c < 8; c++ )
      {
        mez += rd.distance_mm[ (r*8) + c ];
        mez += "\t";
      }
      mez += "\n";
    }
    Serial.println( mez );
    */
    
    if (sleepCnt >= SLEEP_COVERAGE_COUNT) 
    {
      resetGesture();
      direction = GESTURE_SLEEP;
      directionWay = "→ Sleep";
      mymessage = directionWay;
      mymessage2 = sleepCnt;
      captTime = now;
      return;
    }
  }

  if (validCnt < VALID_PIXEL_COUNT) return;

  // rotate+flip into rotated[]
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

  // copy this frame into our circular buffer
  memcpy(rotatedFrames[wi], rotated, sizeof(rotated));

  // compute centroid
  float num = 0, denom = 0;
  for (int i = 0; i < 64; i++) {
    int16_t d = rotated[i];
    if (d >= MIN_DIST_MM && d <= MAX_DIST_MM) {
      float w   = float(MAX_DIST_MM - d);
      num   += w * (i % 8);
      denom += w;
    }
  }
  float centroid = (denom > 0) ? (num / denom) : 3.5f;
  if (centroid == 3.5f) {
    // skip if no object
    return;
  }

  centroidFrames[wi] = centroid;

  // Finger tip location analysis
  if ( centroid < tipMin ) tipMin = centroid;
  if ( centroid > tipMax ) tipMax = centroid;
  tipPos = mapFloatTo0_7( centroid, tipMin, tipMax );
  tipDist = rotated[ ( 8 * 4 ) + tipPos ];

  // compute delta and store
  float delta      = centroid - lastCentroid;
  sumDelta         = sumDelta - deltas[wi] + delta;
  deltas[wi]       = delta;
  lastCentroid     = centroid;

  // Ignore small movements, and when there is enough of them declare a circular gesture
  if (fabs( delta ) < 0.08 )
  {
    /* 
    Serial.print( "ignoring " );
    Serial.println( delta );
    Serial.print( "Cir count " );
    Serial.println( circCnt );
    */

    circCnt++;
    if ( circCnt > CIRCULAR_MAX )
    {
      //Serial.println( cirmesg );
      direction = GESTURE_CIRCULAR;
      directionWay = cirmesg;
      captTime  = now;
      circCnt = 0;
      return;
    }

    return;
  }

  // 
  if ( now - lastValid > MAX_SCANTIME )
  {
    lastValid = now;
    wi = 0;
    for ( int i = 0; i < WINDOW; i++ ) deltas[i] = 0;
  }

  /*
  Serial.print( sumDelta );
  Serial.print( "\t" );
  Serial.print( centroid );
  Serial.print( "\t" );
  Serial.print( delta );
  Serial.print( "\t" );
  Serial.print( wi );
  Serial.print( "\t" );
  
  for ( int i = 0; i < WINDOW; i++ )
  {
    Serial.print( ( wi + i ) % WINDOW );
    Serial.print( " " );
    Serial.print( deltas[ ( wi + i ) % WINDOW ] );
    Serial.print( " " );
  }
  Serial.println( " " );

  */

  // Analysis
  // negative delta = moving right to left
  // positive = moving left to right

  // count consecutive same‐sign deltas
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

  /*
  Serial.print( "R-L neg: " );
  Serial.print( cneg );
  Serial.print( ":" );
  Serial.print( cnegcon );
  Serial.print( " L-R pos: " );
  Serial.print( cpos );
  Serial.print( ":" );
  Serial.println( cposcon );
  */

  // advance write pointer
  wi = (wi + 1) % WINDOW;
  lastValid    = now;

  if ( cnegcon == 0 && cposcon == 0 ) 
  {
    return;
  }

  if ( cneg == 2 && cnegcon == 1 && cpos == 1 && cposcon == 0 )
  {
    //Serial.println( "Ignoring 1" );
    return;
  }
  
  /*
  if ( cneg == 0 && cnegcon == 0 && cpos == 2 && cposcon == 1 )
  {
    //Serial.println( "Ignoring 2" );
    return;
  }
  */
  
  if ( cneg > 0 && cnegcon == 0 && cpos < 3 && cposcon < 2 )
  {
    //Serial.print( lrmesg );
    //Serial.println( "1" );
    direction = GESTURE_LEFT_RIGHT;
    directionWay = lrmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  if ( cneg > 1 && cnegcon > 1 && cpos > 0 && cposcon == 0 )
  {
    //Serial.print( rlmesg );
    //Serial.println( "2" );
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  if ( cneg > 0 && cneg < 3 && cnegcon < 2 && cpos == 0 && cposcon == 0 )
  {
    //Serial.print( rlmesg );
    //Serial.println( "3" );
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  if ( cneg > 0 && cpos > 0 && cneg > cpos && cnegcon > 0 && cposcon > 1 )
  {
    //Serial.print( rlmesg );
    //Serial.println( "4" );
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  if ( cneg > 0 && cpos > 0 && cneg < cpos && cnegcon > 1 && cposcon > 1 )
  {
    //Serial.print( lrmesg );
    //Serial.println( "5" );
    direction = GESTURE_LEFT_RIGHT;
    directionWay = lrmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  if ( cneg > 0 && cpos > 0 && cneg < cpos && cnegcon > 0 && cposcon > 1 )
  {
    //Serial.print( lrmesg );
    //Serial.println( "6" );
    direction = GESTURE_LEFT_RIGHT;
    directionWay = lrmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  if ( cneg > 1 && cnegcon > 0 && cpos > 0 && cposcon == 0 )
  {
    //Serial.print( rlmesg );
    //Serial.println( "7" );
    direction = GESTURE_RIGHT_LEFT;
    directionWay = rlmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

  //Serial.print( "circCnt " );
  //Serial.println( circCnt );
  
  circCnt++;
  if ( circCnt > CIRCULAR_MAX )
  {
    //Serial.println( cirmesg );
    direction = GESTURE_CIRCULAR;
    directionWay = cirmesg;
    captTime  = now;
    circCnt = 0;
    return;
  }

}
