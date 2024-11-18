/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library

 Reflections board usees an (LIS3DHTR 3-Axis Accelerometer) to
 identify user gestures with their wrists and to wake the
 processor from sleep.

*/

#include "TOF.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

//extern Arduino_GFX *gfx;

TOF::TOF(){}

void TOF::begin()
{ 
  started = false;
  
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

  sensor.setRangingFrequency( 15 );

  sensor.startRanging();

  currentSetIndex = 0;

  // Allocate memory for all sets in one go
  buffer = (int16_t*) malloc( NUM_SETS * SET_SIZE * sizeof(int16_t) );
  
  if (buffer == NULL) {
    Serial.println( F( "TOF failed to allocate memory for TOF gesture buffer" ) );
    while (1); // Stop execution if memory allocation fails
  }

  // Initialize the buffer (optional, e.g., set all values to 0)
  memset( buffer, 0, NUM_SETS * SET_SIZE * sizeof(int16_t) );

  fingerTipInRange = false;
  fingerPosRow = 0;
  fingerPosCol = 0;

  for ( int mia = 0; mia < 4; mia++ )
  {
    accumulator[ mia ] = false;
    vertaccumulator[ mia ] = false;
    horizaccumulator[ mia ] = false;
    flyaccumulator[ mia ] = false;
    bombaccumulator[ mia ] = false;
  }

  gestureTime= millis();
  
  recentGesture = TOFGesture::None;

  sleepTimer = millis();
  sleepCount = 0;
  maxCount = 0;
  ssmin = 3000;
  ssmax = 0;
  ssavg = 0;
  ssttl = 0;
  sscnt = 0;

  started = true;
}

bool TOF::tofStatus()
{
  return started;
}

bool TOF::test()
{
  return started;  
}

TOF::TOFGesture TOF::getGesture()
{
  TOF::TOFGesture gs = recentGesture;
  recentGesture = TOF::None;
  return gs;
}

void TOF::detectGestures()
{
  //if ( recentGesture != TOFGesture::None ) return;

  if ( millis() - gestureTime > gestureSensingDuration )
  {
    gestureTime = millis();

    //displayStatus();

    // Clear everything to try again

    for ( int mei = 0; mei < 4; mei++ )
    {
      accumulator[ mei ] = false;
      horizaccumulator[ mei ] = false;
      vertaccumulator[ mei ] = false;
      bombaccumulator[ mei ] = false;
      flyaccumulator[ mei ] = false;
    }
  }

  if ( ! sensor.isDataReady() ) return; 

  if ( ! sensor.getRangingData( &measurementData ) )
  {
    SF_VL53L5CX_ERROR_TYPE errorCode = sensor.lastError.lastErrorCode;

    Serial.println( "TOF sensor error" );
    Serial.println( (int) errorCode );
  }
  
  // Get the pointer to the current set's storage in the buffer
  int16_t* dest = buffer + ( currentSetIndex * SET_SIZE );

  // Copy the measurement data to the buffer
  memcpy( dest, measurementData.distance_mm, SET_SIZE * sizeof( int16_t ) );

  // Move to the next set, wrapping around
  currentSetIndex = ( currentSetIndex + 1 ) % NUM_SETS;

  // Finger tip coordinates used by Pounce, Eyes, Parallax, Horiz, Vert

  detectFingerTip();

  if ( detectSleepGesture() )
  {
    recentGesture = TOFGesture::Sleep;
  }

  if ( ! fingerTipInRange ) return;

  if ( detectCircularGesture() )
  {
    recentGesture = TOFGesture::Circular;
  }

  if ( detectHorizontalGesture() )
  {
    recentGesture = TOFGesture::Horizontal;
  }

  if ( detectVerticalGesture() )
  {
    recentGesture = TOFGesture::Vertical;
  }

  if ( detectBombDropGesture() )
  {
    recentGesture = TOFGesture::BombDrop;
  }

  if ( detectFlyAwayGesture() )
  {
    recentGesture = TOFGesture::FlyAway;
  }

}

/*
  Hold your palm or finger over the sensor to indicate a Sleep gesture
*/

bool TOF::detectSleepGesture() 
{
  if ( millis() - sleepTimer > 5000 )
  {
    sleepTimer = millis();
    sleepCount = 0;
    ssavg = 0;
    ssttl = 0;
    sscnt = 0;
  }

  // Calculate the index of the most recent set
  int recentSetIndex = (currentSetIndex - 1 + NUM_SETS) % NUM_SETS;

  // Get the pointer to the most recent set's storage in the buffer
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  ssavg = 0;
  ssttl = 0;
  sscnt = 0;

  // Process each value in the most recent set
  for ( int mia = 0; mia < SET_SIZE - 9; mia++ )
  {
    int dist = recentSet[ mia ];
    ssttl = ssttl + dist;
    sscnt ++;
  }

  ssavg = ssttl / sscnt;
/*
  Serial.print( "Sleep " );
  Serial.print( sleepCount );
  Serial.print( ", ");
  Serial.print( ssavg );
  Serial.print( ", ");
  Serial.print( sscnt );
  Serial.print( ", ");
  Serial.print( ssttl );
  Serial.print( ", ");
  Serial.println( maxCount );
*/

  if ( ( ssavg > minorityThreshold ) && ( ssavg < majorityThreshold ) )
  {
    sleepCount++;
    if ( sleepCount > sleepRepeat ) return true;
  }

  return false;
}

bool TOF::detectCircularGesture() 
{
  if ( ( fingerPosRow >= 0 ) && ( fingerPosRow < 3 ) )
  {
    // upper quadrant
    if ( ( fingerPosCol >= 0 ) && ( fingerPosCol < 4 ) )
    {
      // upper left
      accumulator[ 0 ] = true;
    }
    else
    {
      // upper right
      accumulator[ 1 ] = true;
    }
  }
  else
  {
    // lower quadrant
    if ( ( fingerPosCol >= 0 ) && ( fingerPosCol < 3 ) )
    {
      // lower right
      accumulator[ 3 ] = true;        // this isn't working!
    }
    else
    {
      // lower left
      accumulator[ 2 ] = true;
    }
  }

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! accumulator[ accx ] ) yahtzee = false;
  }

  if ( yahtzee ) 
  {
    for ( int accx = 0; accx < 4; accx++ )
    {
      accumulator[ accx ] = false;
    }    
    return true;
  }

  return false;
}

bool TOF::detectHorizontalGesture()
{
  if ( fingerTipInRange )
  {
    if ( fingerPosRow == 0 ) horizaccumulator[ 0 ] = true;
    if ( fingerPosRow == 1 ) horizaccumulator[ 0 ] = true;
    if ( fingerPosRow == 2 ) horizaccumulator[ 1 ] = true;
    if ( fingerPosRow == 3 ) horizaccumulator[ 1 ] = true;
    if ( fingerPosRow == 4 ) horizaccumulator[ 2 ] = true;
    if ( fingerPosRow == 5 ) horizaccumulator[ 2 ] = true;
    if ( fingerPosRow == 5 ) horizaccumulator[ 3 ] = true;
    if ( fingerPosRow == 5 ) horizaccumulator[ 3 ] = true;
  }

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! horizaccumulator[ accx ] ) yahtzee = false;
  }

  return yahtzee;
}

bool TOF::detectVerticalGesture()
{
  if ( fingerPosCol == 0 ) vertaccumulator[ 0 ] = true;
  if ( fingerPosCol == 1 ) vertaccumulator[ 0 ] = true;
  if ( fingerPosCol == 2 ) vertaccumulator[ 1 ] = true;
  if ( fingerPosCol == 3 ) vertaccumulator[ 1 ] = true;
  if ( fingerPosCol == 4 ) vertaccumulator[ 2 ] = true;
  if ( fingerPosCol == 5 ) vertaccumulator[ 2 ] = true;
  if ( fingerPosCol == 6 ) vertaccumulator[ 3 ] = true;
  if ( fingerPosCol == 7 ) vertaccumulator[ 3 ] = true;

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! vertaccumulator[ accx ] ) yahtzee = false;
  }

  return yahtzee;
}

bool TOF::detectBombDropGesture()
{
  bombaccumulator[ map( fingerDist, bombFlyDistLow, bombFlyDistHigh, 0, 3 ) ] = true;

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! bombaccumulator[ accx ] ) yahtzee = false;
  }

  return yahtzee;
}

bool TOF::detectFlyAwayGesture()
{
  flyaccumulator[ map( fingerDist, bombFlyDistLow, bombFlyDistHigh, 0, 3 ) ] = true;

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! flyaccumulator[ accx ] ) yahtzee = false;
  }

  return yahtzee;
}

bool TOF::detectFingerTip() 
{
  closeReadingsCount = 0;
  maxCount = 0;
  fingerTipInRange = false;

  // Calculate the index of the most recent set
  int recentSetIndex = (currentSetIndex - 1 + NUM_SETS) % NUM_SETS;

  // Get the pointer to the most recent set's storage in the buffer
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  // Process each value in the most recent set
  for ( int mia = 0; mia < SET_SIZE - 9; mia++ )
  {
    int dist = recentSet[ mia ];
    int dist2 = recentSet[ mia + 1 ];
    int dist3 = recentSet[ mia + 8 ];
    int dist4 = recentSet[ mia + 9 ];

    if ( ( dist > fingerDetectionThresholdLow ) && ( dist < fingerDetectionThresholdHigh ) )
    {       
      if ( (dist <= dist2 + fingerWiggleRoom ) && (dist >= dist2 - fingerWiggleRoom ) )
      {                                
        if ( (dist <= dist3 + fingerWiggleRoom ) && (dist >= dist3 - fingerWiggleRoom ) )
        {
          if ( (dist <= dist4 + fingerWiggleRoom ) && (dist >= dist4 - fingerWiggleRoom ) )
          {
            // Sensor doesn't have a complete field of vision, columns and rows 6 and 7 are too far to the right
            // If the original value (from index % 8) is between 0 and 5, it is scaled to a value 
            // between 0 and 7 using the formula (originalColumn * 7) / 5.
            // If the original value is 6 or 7, it maps directly to 7.

            fingerPosRow = mia >> 3;
/*
            if (fingerPosRow < 6) 
            {
              fingerPosRow = (fingerPosRow * 7) / 5;
            }
            else
            {
              // If original is 6 or 7, it maps directly to 7
              fingerPosRow = 7;
            }
*/
            // Same scaling for columns

            fingerPosCol = mia % 8;
/*
            if (fingerPosCol < 6) 
            {
              fingerPosCol = (fingerPosCol * 7) / 5;
            }
            else
            {
              // If originalColumn is 6 or 7, it maps directly to 7
              fingerPosCol = 7;
            }
*/

            fingerDist = dist;
            fingerTipInRange = true;
            return true;
          }
        }
      }
    }
  }

  return false;
}

/* Pretty print sensor measurements */

void TOF::printTOF()
{
  if ( ! started ) return;

  if ( sensor.isDataReady() == true )
  {
    if ( sensor.getRangingData( &measurementData ) ) //Read distance data into array
    {
      closeReadingsCount = 0;

      for (int y = 0; y < GRID_ROWS; y++)
      {
        for ( int x = 0; x < GRID_COLS; x++)
        {
          float dist = measurementData.distance_mm[ ( x * GRID_COLS ) + y ];

          if ( dist > tofmaxdist )
          {
            Serial.print( F( "\t" ) );
            Serial.print( "--" );
          }
          else
          {
            Serial.print( F( "\t" ) );
            Serial.print( dist );
          }
        }
        Serial.println( " " );
      }
      Serial.println( " " );
    }
  }
}

void TOF::showBubbles()
{
  if ( ! sensor.isDataReady() ) return; 

  sensor.getRangingData( &measurementData );

  for (int y = 0; y < GRID_ROWS; y++)
  {
    for ( int x = 0; x < GRID_COLS; x++)
    {
      //gfx -> fillCircle( xspace + ( y * ydistance ), xspace + ( x * xdistance ), tofdiam, COLOR_BACKGROUND);

      float dist = measurementData.distance_mm[ x + ( y * GRID_COLS ) ];

      if ( dist > tofmaxdist )
      {
        dist = tofmaxdist;
      }
      
      int diam2 = (int) ( ( (float) tofdiam ) * ( 1- ( dist / tofmaxdist ) ) );
      if ( diam2 > 0 )
      {
        //gfx -> drawCircle( yspace + ( y * ydistance ), xspace + ( x * xdistance ), diam2, COLOR_RING);
      }
    }
  }
}

void TOF::displayStatus()
{
  if (!started) return;

  String mef = " ";

  mef += "Finger: (";
  mef += fingerPosRow;
  mef += ", ";
  mef += fingerPosCol;
  mef += ") | ";

  // Circular gesture accumulators
  mef += "Circ: [";
  for (int i = 0; i < 4; i++) {
      mef += accumulator[i] ? "1" : "0";
      if (i < 3) mef += ", ";
  }
  mef += "] | ";

  // Horizontal gesture accumulators
  mef += "Horiz: [";
  for (int i = 0; i < 4; i++) {
      mef += horizaccumulator[i] ? "1" : "0";
      if (i < 3) mef += ", ";
  }
  mef += "] | ";

  // Vertical gesture accumulators
  mef += "Vert: [";
  for (int i = 0; i < 4; i++) {
      mef += vertaccumulator[i] ? "1" : "0";
      if (i < 3) mef += ", ";
  }
  mef += "] | ";

  // BombDrop gesture accumulators
  mef += "Bomb: [";
  for (int i = 0; i < 4; i++) {
      mef += bombaccumulator[i] ? "1" : "0";
      if (i < 3) mef += ", ";
  }
  mef += "] | ";

  // FlyAway gesture accumulators
  mef += "Fly: [";
  for (int i = 0; i < 4; i++) {
      mef += flyaccumulator[i] ? "1" : "0";
      if (i < 3) mef += ", ";
  }
  mef += "], Sleep ";
  mef += String( sleepCount );

  mef += ", index ";
  mef += String( currentSetIndex );

  Serial.println( mef );
}

void TOF::loop()
{
  if ( ! started ) return;

  detectGestures();
}
