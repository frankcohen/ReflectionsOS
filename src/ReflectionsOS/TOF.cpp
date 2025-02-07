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
 the cat watch going to sleep to operating the Experiences.
 
*/

#include "TOF.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

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

  sleepCount = 0;

  previousMillis = millis();

  myMef = "";
  myMef2 = "";

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
//  if ( recentGesture != TOFGesture::None ) return;

  fingerTipInRange = detectFingerTip( currentSetIndex );

return;

  if ( detectSleepGesture() )
  {
    // recentGesture = TOFGesture::Sleep;
  }

  if ( detectLeftToRight() )
  {
    //recentGesture = TOFGesture::Horizontal;
    Serial.println("TOF left-to-right");
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
  Move finger from left to right in 2.5 seconds
*/

#define leftToRightFrames 10          // How many frames back to analyze
#define leftToRightPercentage 0.50    // Percentage of hits to be considered a gesture


bool TOF::detectLeftToRight()
{
  float count = 0;
  int lrpos = GRID_ROWS;

  //myMef2 = "detectLeftToRight: ";

  for ( int i = 0; i < leftToRightFrames; i++ )
  {
    if ( detectFingerTip( currentSetIndex - i ) )
    {
/*      myMef2 += "^";
      myMef2 += currentSetIndex;
      myMef2 += " ";
      myMef2 += i;
      myMef2 += " ";
      myMef2 += count;
      myMef2 += " ";
      myMef2 += fingerPosRow;
      myMef2 += " ";
      myMef2 += lrpos;
      myMef2 += "@ ";
      */

      //if ( fingerPosRow < lrpos )
      //{
        count++;
        lrpos = fingerPosRow;
      //}      
    }
  }

  bool rlf = false;

  if ( ( count / leftToRightFrames ) > leftToRightPercentage ) rlf = true;

/*
  myMef2 += String( count / leftToRightFrames );
  myMef2 += ", ";
  myMef2 += rlf;
*/

  return rlf;
}

/*
  Hold palm or finger over the sensor for a Sleep gesture
*/

bool TOF::detectSleepGesture() 
{
  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( currentSetIndex - 1 + NUM_SETS ) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  // Count how many values are below the filter

  float count = 0;
  float mval = 0;

  for ( int mia = 0; mia < SET_SIZE; mia++ )
  {
    mval = recentSet[ mia ];
    if ( ( mval > sleepLowFilter ) && ( mval < sleepHighFilter ) ) count++;
  }

  if ( ( count / SET_SIZE ) > sleepPercentage )
  {
    return true;
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

// Finger tip coordinates used by Pounce, Eyes, Parallax, Horiz, Vert

bool TOF::detectFingerTip( int setnum ) 
{
  fingerTipInRange = false;

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( setnum - 1 + NUM_SETS) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  // Find the center of the boundary within the sensor values

  int minX = GRID_ROWS, maxX = -1, minY = GRID_COLS, maxY = -1;

  float dist = 0;

  for (int row = 0; row < GRID_ROWS; row++) 
  {
    for (int col = 0; col < GRID_COLS; col++) 
    {
      dist = recentSet[ ( col * GRID_COLS ) + row ];

      if ( ( dist > fingerDetectionThresholdLow ) && ( dist < fingerDetectionThresholdHigh ) )
      {
        // Update the bounds
        minX = min(minX, col);
        maxX = max(maxX, col);
        minY = min(minY, row);
        maxY = max(maxY, row);
      }
    }
  }

  // If there are non-zero cells, calculate the center of the bounding box
  if ( ( minX <= maxX ) && ( minY <= maxY ) ) 
  {
    fingerPosRow = (minX + maxX) / 2;
    fingerPosCol = (minY + maxY) / 2;

    fingerDist = recentSet[ ( fingerPosCol * GRID_COLS ) + fingerPosRow ];
    fingerTipInRange = true;

    // Color the center point

    int spotdiam = round( ( ( fingerDist - fingerDetectionThresholdLow ) / ( fingerDetectionThresholdHigh - fingerDetectionThresholdLow ) ) * tofdiam );

    gfx -> fillCircle( yspace + ( fingerPosRow * ydistance ), xspace + ( fingerPosCol * xdistance ), spotdiam, COLOR_BLUE);

    return true;
  }
  else
  {
    return false;
  }
}

/* Pretty print sensor measurements */

void TOF::printTOF()
{
  if ( ! started ) return;

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

void TOF::displayStatus()
{
  Serial.println( getStats() );
}

String TOF::getStats()
{
  if (!started) return " ";

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

  return mef;
}

String TOF::getMef()
{
  return myMef;
}

String TOF::getMef2()
{
  return myMef2;
}

void TOF::acquireDataToBuffer()
{
  if ( ! sensor.getRangingData( &measurementData ) )
  {
    SF_VL53L5CX_ERROR_TYPE errorCode = sensor.lastError.lastErrorCode;
    Serial.println( "TOF sensor error" );
    Serial.println( (int) errorCode );
    return;
  }
  
  // Get the pointer to the current set's storage in the buffer
  int16_t* dest = buffer + ( currentSetIndex * SET_SIZE );

  // Copy the measurement data to the buffer
  memcpy( dest, measurementData.distance_mm, SET_SIZE * sizeof( int16_t ) );

  // Move to the next set, wrapping around
  currentSetIndex = ( currentSetIndex + 1 ) % NUM_SETS;

  // Filter out the invalid readings

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = (currentSetIndex - 1 + NUM_SETS) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  for ( int mia = 0; mia < SET_SIZE; mia++ )
  {
    int row = mia / 8;
    int col = mia % 8;
    float spot = recentSet[ mia ];

    gfx -> fillCircle( xspace + ( row * xdistance ), yspace + ( col * ydistance ), tofdiam, COLOR_BACKGROUND);

    if ( ( spot < closefilter ) || ( spot > farfilter ) ) 
    {
      recentSet[ mia ] = 0.0;
    }
    else
    {
      // Show bubbles

      int spotdiam = round( ( ( spot - closefilter ) / ( farfilter - closefilter ) ) * tofdiam );

      if ( spotdiam > 0 )
      {
        gfx -> drawCircle( xspace + ( row * xdistance ), yspace + ( col * ydistance ), spotdiam, COLOR_RING);
      }
    }

  }
}

void TOF::loop()
{
  if ( ! started ) return;

  if ( millis() - previousMillis > 250 ) 
  {
    previousMillis = millis();

    acquireDataToBuffer();    // And show bubbles

    detectGestures();
  }
}
