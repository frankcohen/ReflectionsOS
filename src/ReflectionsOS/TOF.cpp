/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library

 Reflections board usees a Time Of Flight (TOF) VL53L5CX sensor to
 identify user gestures with their fingers and hand. Gestures control
 operating the Experiences.
 
 Datasheet comes with this source code, see: vl53l5cx-2886943_.pdf

*/

#include "TOF.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

TOF::TOF(){}

void TOF::begin()
{ 
  started = false;
  paused = true;

  if ( sensor.begin( 0x29, Wire ) ) 
  {
    logger.info( F( "TOF sensor started" ) );
  }
  else
  {
    logger.info( F( "TOF sensor failed to initialize" ) );
    while(1);
  }

  sensor.setResolution( 8*8 ); //Enable all 64 pads

  //Using 4x4, min frequency is 1Hz and max is 60Hz
  //Using 8x8, min frequency is 1Hz and max is 15Hz
  if ( sensor.setRangingFrequency( 4 ) )
  {
    /*
    int frequency = sensor.getRangingFrequency();
    if (frequency > 0)
    {
      Serial.print(F("TOF ranging frequency set to "));
      Serial.print(frequency);
      Serial.println(F(" Hz."));
    }
    else
      Serial.println( F( "TOF error getting ranging frequency." ));
    */
  }
  else
  {
    Serial.println( F( "Cannot set TOF frequency requested. Stopping."));
    while (1) ;
  }

  sensor.startRanging();

  // Allocate memory for all sets in one go
  tofbuffer = (int16_t*) malloc( ( NUM_SETS + 4 ) * SET_SIZE * sizeof(int16_t) );
  
  if (tofbuffer == NULL) {
    Serial.println( F( "TOF failed to allocate memory for TOF gesture buffer" ) );
    while (1); // Stop execution if memory allocation fails
  }

  resetBuffer();

  fingerTipInRange = false;
  fingerPosRow = 0;
  fingerPosCol = 0;

  lastSampleTime = 0;
  hoverStartTime = 0;
  baselineRow = 0;
  baselineCol = 0;
  baselineDist = 0.0;
    
  for ( int mia = 0; mia < 4; mia++ )
  {
    flyaccumulator[ mia ] = false;
    bombaccumulator[ mia ] = false;
  }

  debouncetime = millis();

  gestureTime= millis();
  
  recentGesture = TOFGesture::None;

  sleepCount = 0;

  previousMillis = millis();

  myMef = F("");
  myMef2 = F("");

  for (int i = 0; i<8; i++ )
  {
    previousHorizPositions[i] = -1;
  }

  started = true;
}

/* True once the sensor starts */

bool TOF::tofStatus()
{
  return started;
}

bool TOF::getPaused()
{
  return paused;
}

void TOF::startGestureSensing()
{
  paused = false;
}

void TOF::stopGestureSensing()
{
  paused = true;
}

bool TOF::test()
{
  return started;  
}

TOF::TOFGesture TOF::getGesture()
{
  TOF::TOFGesture gs = recentGesture;
  recentGesture = TOFGesture::None;
  return gs;
}

void TOF::setStatus( TOFGesture status )
{
  recentGesture = status;
}

/* Returns a String of the recent gesture, or None, does not clear the gesture */

String TOF::getGestureName()
{
    String gestureName = F("");

    // Map the gesture enum to a string
    switch ( recentGesture )
    {
        case None:
            gestureName = F("None");
            break;
        case Sleep:
            gestureName = F("Sleep");
            break;
        case Circular:
            gestureName = F("Circular");
            break;
        case Right:
            gestureName = F("Right");
            break;
        case Left:
            gestureName = F("Left");
            break;
        case Up:
            gestureName = F("Up");
            break;
        case Down:
            gestureName = F("Down");
            break;
        default:
            gestureName = String( recentGesture );
            //gestureName = F("Unknown");
            break;
    }

    return gestureName;
}

/*
  Hold palm or finger over the sensor for a Sleep gesture
*/

bool TOF::detectSleepGesture() 
{
  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( currentSetIndex - 1 + NUM_SETS ) % NUM_SETS;
  int16_t* recentSet = tofbuffer + ( recentSetIndex * SET_SIZE );

  // Count how many values are below the filter

  float scount = 0;
  int16_t mval = 0;

  for ( int mia = 0; mia < SET_SIZE; mia++ )
  {
    mval = recentSet[ mia ];
    if ( ( mval > sleepLowFilter ) && ( mval < sleepHighFilter ) ) scount++;
  }
  
  if ( ( scount / SET_SIZE ) > sleepPercentage )
  {
    return true;
  }

  return false;
}

void TOF::resetBuffer()
{
  // Initialize the buffer (optional, e.g., set all values to 0)
  memset( tofbuffer, 0, NUM_SETS * SET_SIZE * sizeof(int16_t) );

  currentSetIndex = 0;
}

/* Fab5 could be one of these:
   Salute to Queer Eye for the Straight Guy, a wonderful entertainment
   Honoring Mickey, Minnie, Donald, Goofy and Pluto
   Gestures for left, right, up, down, and circular movement
*/

bool TOF::detectFab5Gestures() 
{
  int leftMovement = 0;
  int rightMovement = 0;

  int upMovement = 0;
  int downMovement = 0;

  for (int i = 0; i < ( movementFrames - 1); i++) 
  {
    // Calculate indices for the newer and older frames in the pair.
    int indexNew = (currentSetIndex - 1 - i + NUM_SETS) % NUM_SETS;
    int indexOld = (currentSetIndex - 2 - i + NUM_SETS) % NUM_SETS;

    int16_t* newFrame = tofbuffer + (indexNew * SET_SIZE);
    int16_t* oldFrame = tofbuffer + (indexOld * SET_SIZE);

    myMef2 = F("");

    // Loop through each row
    for (int row = 0; row < 7; row++) 
    {
      for (int col = 0; col < 8; col++) 
      {
        int16_t new1 = newFrame[ (row * 8 ) + col ];
        int16_t old1 = oldFrame[ (row * 8 ) + col + 1];

        int16_t new2 = newFrame[ (row * 8 ) + col + 1 ];
        int16_t old2 = oldFrame[ (row * 8 ) + col ];

        if ( ( ( new1 > movementLow ) && ( new1 < movementHigh ) )
          && ( ( old1 > movementLow ) && ( old1 < movementHigh ) ) )
        {
          leftMovement++;
        }

        if ( ( ( new2 > movementLow ) && ( new2 < movementHigh ) )
          && ( ( old2 > movementLow ) && ( old2 < movementHigh ) ) )
        {
          rightMovement++;
        }

        if ( row > 0 )
        {
          new1 = newFrame[ ( row * 8 ) + col ];
          old1 = oldFrame[ ( ( row + 1 ) * 8 ) + col ];

          new2 = newFrame[ ( ( row + 1 ) * 8 ) + col ];
          old2 = oldFrame[ ( row * 8 ) + col ];

          if ( ( ( new1 > movementLow ) && ( new1 < movementHigh ) )
            && ( ( old1 > movementLow ) && ( old1 < movementHigh ) ) )
          {
            upMovement++;
          }

          if ( ( ( new2 > movementLow ) && ( new2 < movementHigh ) )
            && ( ( old2 > movementLow ) && ( old2 < movementHigh ) ) )
          {
            downMovement++;
          }        
        }
      }
    }
  }

  // Return direction based on counts
  
  if ( ( leftMovement >= circularCountLow ) || ( rightMovement >= circularCountLow ) )
  {
    recentGesture = TOFGesture::Circular;

    myMef = F("Circular ");
    myMef += leftMovement;    
    myMef += F(" ");
    myMef += rightMovement;
    resetBuffer();
    return true;
  }

  if ( ( leftMovement > 2 ) || ( rightMovement > 2 )
  && ( abs( leftMovement - rightMovement ) > 2 ) )
  {
    if ( leftMovement > rightMovement ) 
    {
      recentGesture = TOFGesture::Left;
      myMef = F("Left ");
      myMef += leftMovement;    
      myMef += F(" ");
      myMef += rightMovement;
      resetBuffer();
      return true;
    } 
    
    else if ( rightMovement > leftMovement) 
    {
      recentGesture = TOFGesture::Right;
      myMef = F("Right ");
      myMef += leftMovement;    
      myMef += F(" ");
      myMef += rightMovement;
      resetBuffer();
      return true;
    }
  }

  if ( ( upMovement > 2 ) || ( downMovement > 2 ) 
  && ( abs( upMovement - downMovement ) < 3 ) )
  {
    if ( upMovement > downMovement ) 
    {
      recentGesture = TOFGesture::Up;
      myMef = F("Up ");
      myMef += upMovement;    
      myMef += F(" ");
      myMef += downMovement;
      resetBuffer();
      return true;
    } 
    
    else if ( downMovement > upMovement ) 
    {
      recentGesture = TOFGesture::Down;
      myMef = F("Down ");
      myMef += upMovement;    
      myMef += F(" ");
      myMef += downMovement;
      resetBuffer();
      return true;
    } 
  }

  myMef = F("");
  return false;
}

// Find Finger tip coordinates from a stored set of sensor measurements

bool TOF::detectFingerTip( int setnum ) 
{
  if ( ( setnum < 0 ) || ( setnum > NUM_SETS ) ) return false;

  fingerTipInRange = false;
  fingerPosRow = 0;
  fingerPosCol = 0;

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( setnum - 1 + NUM_SETS) % NUM_SETS;
  int16_t* recentSet = tofbuffer + ( recentSetIndex * SET_SIZE );

  // Find the center of the boundary within the sensor values

  int minX = GRID_ROWS, maxX = -1, minY = GRID_COLS, maxY = -1;

  float dist = 0;

  int foundcount = 0;
  
  for ( int y = 0; y < 8; y++ )
  {
    for ( int x = 0; x < 8; x++ )
    {
      int16_t offset = (y * 8) + x;
      int16_t dist = recentSet[ offset ];

      if ( ( dist > fingerDetectionThresholdLow ) && ( dist < fingerDetectionThresholdHigh ) )
      {
        foundcount ++;
        
        // Update the bounds
        minX = min(minX, y);
        maxX = max(maxX, y);
        minY = min(minY, x);
        maxY = max(maxY, x);
      }
    }
  }

  if ( foundcount < 4 ) return false;

  // If there are non-zero cells, calculate the center of the bounding box
  if ( ( minX <= maxX ) && ( minY <= maxY ) ) 
  {
    fingerPosRow = (minX + maxX) / 2;
    fingerPosCol = (minY + maxY) / 2;

    fingerDist = recentSet[ ( fingerPosCol * GRID_COLS ) + fingerPosRow ] ;
    fingerTipInRange = true;

    // Color the finger center point
    
    /*
      int16_t xloc = xspace + ( fingerPosCol * xdistance );
      int16_t yloc = yspace + ( fingerPosRow * ydistance );
      gfx -> fillCircle( xloc, yloc, tofdiam, COLOR_BLUE);
    */

    return true;
  }
  else
  {
    return false;
  }
}

/* Pretty print sensor measurements
   Does not correct for sensor lense reversing values, use flipAndRotateArray()
 */

String TOF::getRawMeasurements()
{
  if ( ! started ) return F("");

  rawMeasurements = F("");
  
  if ( sensor.isDataReady() == true )
  {
    if ( sensor.getRangingData( &measurementData ) ) //Read distance data into array
    {
      for (int y = 0; y < GRID_ROWS; y++)
      {
        for ( int x = 0; x < GRID_COLS; x++)
        {
          float dist = measurementData.distance_mm[ ( x * GRID_COLS ) + y ];

          if ( dist > tofmaxdist )
          {
            rawMeasurements += F( "\t" );
            rawMeasurements += F( "--" );
          }
          else
          {
            rawMeasurements += F( "\t" );
            rawMeasurements += dist;
          }
        }
        rawMeasurements += F("\n");
      }
      rawMeasurements += F("\n");
    }
  }
  return rawMeasurements;
}

/* Used to send debug information to the Serial Monitor
   TOF operates in Core 0 in parallel to the Arduino code
   running the Serial Monitor in Core 1. Using Serial.println( F("hi") )
   will often be clipped or ignored when run in Core 0. The main
   loop() in ReflectionsOS.ino calls TOF::getMef() and prints to
   Serial Monitor from there. */

String TOF::getRecentMessage()
{
  String myMefa = myMef;
  myMef = F("");
  return myMefa;
}

String TOF::getRecentMessage2()
{
  String myMefa = myMef2;
  myMef2 = F("");
  return myMefa;
}

int TOF::getFingerPosRow()
{
  return fingerPosRow;
}

int TOF::getFingerPosCol()
{
  return fingerPosCol;
}

float TOF::getFingerDist()
{
  return fingerDist;
}

/* Adjusts sensor data to make up for VL53L5CX sensor aiming lense. */

void TOF::flipAndRotateArray(int16_t* dest, int width, int height) 
{

  /* VL53L5CX sensor aiming lense reverses sensor data. I do not
   understand why the sensor data needs to be flipped horizontally
   and then rotated 90 degreeS BUT does NOT also need to be flipped 
   vertically. I decided to move on. 
   
  // Flip vertically (reverse the order of rows)
  for (int row = 0; row < height; ++row) {
      for (int col = 0; col < width / 2; ++col) {
          int leftIdx  = row * width + col;
          int rightIdx = row * width + (width - 1 - col);
          float temp = dest[leftIdx];
          dest[leftIdx] = dest[rightIdx];
          dest[rightIdx] = temp;
      }
  }
  */

  // Flip horizontally (reverse each row)
  for (int row = 0; row < height / 2; ++row) {
      for (int col = 0; col < width; ++col) {
          int topIdx    = row * width + col;
          int bottomIdx = (height - 1 - row) * width + col;
          float temp = dest[topIdx];
          dest[topIdx] = dest[bottomIdx];
          dest[bottomIdx] = temp;
      }
  }

  // Rotate 90° clockwise in place.
  // For a square matrix, rotate in layers.
  int n = width; // (width == height == 8)
  for (int i = 0; i < n / 2; ++i) 
  {
    for (int j = i; j < n - i - 1; ++j) 
    {
      float temp = dest[i * n + j];
      // Move element from left -> top
      dest[i * n + j] = dest[(n - j - 1) * n + i];
      // Move element from bottom -> left
      dest[(n - j - 1) * n + i] = dest[(n - i - 1) * n + (n - j - 1)];
      // Move element from right -> bottom
      dest[(n - i - 1) * n + (n - j - 1)] = dest[j * n + (n - i - 1)];
      // Move temp (top) -> right
      dest[j * n + (n - i - 1)] = temp;
    }
  }
}

/* Check for valid buffer */

bool TOF::checkBuffer()
{
  // Check if buffer and measurementData are valid
  if ( tofbuffer == NULL ) 
  {
    Serial.println(F("TOF Error: buffer is null"));
    return false;
  }

  if ( measurementData.distance_mm == NULL ) 
  {
    Serial.println(F("TOF Error: measurementData.distance_mm is null"));
    return false;
  }

  // Ensure currentSetIndex is within bounds for the buffer
  if ( currentSetIndex >= 100 ) 
  {
    Serial.println(F("Error: currentSetIndex is out of bounds"));
    return false;
  }

  return true;
}

/* Store VL53L5CX sensor data into circular buffer */

void TOF::acquireDataToBuffer()
{
  if ( ! sensor.isDataReady() ) return;

  if ( ! sensor.getRangingData( &measurementData ) )
  {
    SF_VL53L5CX_ERROR_TYPE errorCode = sensor.lastError.lastErrorCode;
    Serial.println( F("TOF sensor error") );
    Serial.println( (int) errorCode );
    return;
  }

  if ( ! checkBuffer() ) return;

  // Get the pointer to the current set's storage in the buffer
  int16_t* dest = tofbuffer + ( currentSetIndex * SET_SIZE );

  // Copy the measurement data to the buffer
  memcpy( dest, measurementData.distance_mm, SET_SIZE * sizeof( int16_t ) );

  flipAndRotateArray( dest, 8, 8 );

  // Move to the next set, wrapping around
  currentSetIndex = ( currentSetIndex + 1 ) % NUM_SETS;

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = (currentSetIndex - 1 + NUM_SETS) % NUM_SETS;
  int16_t* recentSet = tofbuffer + ( recentSetIndex * SET_SIZE );

  // Filter out the invalid readings

  for ( int mia = 0; mia < SET_SIZE; mia++ )
  {
    if ( ( recentSet[ mia ] < closefilter ) || ( recentSet[ mia ] > farfilter ) ) 
    {
      recentSet[ mia ] = 0;
    }
  }

  // Debugging helpers, enable as you wish
  
  /*
    myMef2 = F("Bubbles: ");
    for ( int j = 0; j < 64; j++ )
    {
      myMef2 += recentSet[ j ];
      myMef2 += F(", ");
    }
    myMef2 += F("\n#");
  */

  /* Show bubbles on display
     Displays 8 bubbles in 8 rows. Bubbles are sized proportional to the
     distance from the sensor, and filtered to distances between
     bubleHigh and bubbleLow
  /*

  /*
    for ( int y = 0; y < 8; y++ )
    {
      for ( int x = 0; x < 8; x++ )
      {
        int16_t xloc = xspace + ( x * xdistance );
        int16_t yloc = yspace + ( y * ydistance );
        int16_t offset = (y * 8) + x;
        int16_t spot = recentSet[ offset ];

        gfx -> fillCircle( xloc, yloc, tofdiam, COLOR_BACKGROUND);

        if ( ( spot > bubbleLow ) && ( spot < bubbleHigh ) ) 
        {
          float spot1 = spot - bubbleLow;
          float spot2 = bubbleHigh - bubbleLow ;
          float spot3 = spot1 / spot2;
          int16_t spotdiam = round( spot3 * tofdiam );

          if ( spotdiam > 0 )
          {
            gfx -> drawCircle( xloc, yloc, spotdiam, COLOR_RING);
          }
        }
      }
    }
  */

}

void TOF::detectHover()
{
  unsigned long currentTime = millis();

  if ( currentTime - lastSampleTime < HOVER_SAMPLE_INTERVAL_MS ) return;
  lastSampleTime = currentTime;
        
  // If the finger tip is detected:
  if ( ! fingerTipInRange ) return;

  // If this is the first sample of a possible hover, record the baseline.
  if ( hoverStartTime == 0 ) 
  {
    hoverStartTime = currentTime;
    baselineRow = fingerPosRow;
    baselineCol = fingerPosCol;
    baselineDist = fingerDist;
  }
  
  // Check if the current reading is within ±15% of the baseline.
  bool stable = true;
  if (fabs(fingerPosRow - baselineRow) > 0.15 * fabs(baselineRow))
    stable = false;
  if (fabs(fingerPosCol - baselineCol) > 0.15 * fabs(baselineCol))
    stable = false;
  if (fabs(fingerDist - baselineDist) > 0.15 * fabs(baselineDist))
    stable = false;
  
  // If the reading deviates too much, reset the baseline and timer.
  if ( ! stable ) 
  {
    hoverStartTime = currentTime;
    baselineRow = fingerPosRow;
    baselineCol = fingerPosCol;
    baselineDist = fingerDist;
  }

  // If the reading has been stable for at least 4 seconds, trigger the gesture.
  else if (currentTime - hoverStartTime >= HOVER_DURATION_MS) 
  {
    Serial.println(F("Hover gesture detected for 4 seconds"));
    recentGesture = TOFGesture::Hover;
    hoverStartTime = currentTime;  // or set to 0 if you want to start fresh next time
    return;
  } 

  // If no finger is detected, reset the hover timer.
  else 
  {
    hoverStartTime = 0;
  }
}

void TOF::loop()
{
  if ( ! started ) return;

  if ( millis() - previousMillis < 250 ) return;
  previousMillis = millis();

  if ( ! checkBuffer() ) return;

  acquireDataToBuffer();    // And optionally show bubbles

  detectFingerTip( currentSetIndex );

  if ( paused ) return;

  if ( recentGesture != TOFGesture::None ) return;    // One gesture at a time

  detectFab5Gestures();

  detectHover();

  detectSleepGesture();
}
