/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library
*/

#include "TOF.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Arduino_GFX *gfx;

TOF::TOF(){}

void TOF::begin()
{ 
  started = false;
  
  if ( sensor.begin( 0x29, Wire ) ) 
  {
    logger.info( F( "TOF sensor started" ) );

    sensor.setResolution( 8*8 ); //Enable all 64 pads

    sensor.setRangingFrequency( CAPTURE_RATE );

    sensor.startRanging();

    started = true;
  }
  else
  {
    logger.info( F( "TOF sensor failed to initialize" ) );
    while(1);
  }

  gestureBuffer = (GestureData*) malloc( TOF_BUFFER_SIZE * sizeof( GestureData ) );

  if (gestureBuffer == nullptr) 
  {
    Serial.println( F( "TOF failed to allocate memory for TOF gesture buffer" ) );
    while (1); // Halt execution if allocation fails
  }

  memset( gestureBuffer, 0, TOF_BUFFER_SIZE * sizeof( GestureData ) );

  currentBlockID = 0;

  fingerTime = millis();
  fingerTipInRange = false;
  fingerPosRow = 0;
  fingerPosCol = 0;

  for ( int mia = 0; mia < 4; mia++ )
  {
    accumulator[ mia ] = false;
    accumulator[ mia ] = false;
    bombaccumulator[ mia ] = false;
    vertaccumulator[ mia ] = false;
    horizaccumulator[ mia ] = false;
    flyaccumulator[ mia ] = false;
  }

  circularTimeOut  = millis();
  horizTimeOut = millis();
  vertTimeOut = millis();
  bombTimeOut = millis();
  flyTimeOut = millis();

  lastPollTime = millis();
  delayTime = millis();
  paceTime = millis();

  recentGesture = TOFGesture::None;  
}

bool TOF::tofStatus()
{
  return started;
}

bool TOF::test()
{
  return started;  
}

int TOF::getGesture()
{
  int gs = recentGesture;
  recentGesture = TOFGesture::None;
  return gs;
}

/*
Gets most recent buffer entries, including wrapping around

Example usage:
void exampleUsage() {
    const int recentCount = 5;
    GestureData recentValues[recentCount];
    getMostRecentValues(recentValues, recentCount);
    
    // Print the most recent 5 values
    for (int i = 0; i < recentCount; i++) 
    {
      Serial.print("Value ");
      Serial.print(i);
      Serial.print(": x=");
      Serial.print(recentValues[i].x);
    }

*/

void TOF::getMostRecentValues(GestureData recentValues[], int count)
{
  int startIndex = (bufferIndex - count + BUFFER_SIZE) % BUFFER_SIZE;
  for (int i = 0; i < count; i++) 
  {
    int index = (startIndex + i) % BUFFER_SIZE;
    recentValues[i] = gestureBuffer[index];
  }
}

void TOF::detectGestures()
{
  if ( recentGesture != TOFGesture::None ) return;

  if ( millis() - delayTime < 180 ) return;
  delayTime = millis();

  if ( ! sensor.isDataReady() ) return; 

  int distance;

  sensor.getRangingData( &measurementData );

  for ( int angela = 0; angela < 8; angela++ ) 
  {
    for ( int heather = 0; heather < 8; heather++ ) 
    {
      distance = measurementData.distance_mm[ heather + ( angela * 8 ) ];
      gestureBuffer[ bufferIndex ] = { distance, currentBlockID };
      bufferIndex = ( bufferIndex + 1 ) % BUFFER_SIZE;
    }
  }

  currentBlockID++;
  
  // Finger tip coordinates used by Pounce, Eyes, Parallax

  if ( detectFingerTip() )
  {
    Serial.print("TOF Finger tip row " );
    Serial.print( fingerPosRow );
    Serial.print( " column ");
    Serial.println( fingerPosCol );
  }

  if ( detectSleepGesture() )
  {
    Serial.println( "TOF Sleep gesture detected" );
    recentGesture = TOFGesture::Sleep;
  }

  if ( ! fingerTipInRange ) return;

  if ( detectCircularGesture() )
  {
    Serial.println( "TOF Circular gesture detected" );
    recentGesture = TOFGesture::Circular;
  }

  if ( detectHorizontalGesture() )
  {
    Serial.println( "TOF Horizontal gesture detected" );
    recentGesture = TOFGesture::Horizontal;
  }

  if ( detectVerticalGesture() )
  {
    Serial.println( "TOF Vertical gesture detected" );
    recentGesture = TOFGesture::Vertical;
  }

  if ( detectBombDropGesture() )
  {
    Serial.println( "TOF BombDrop gesture detected" );
    recentGesture = TOFGesture::BombDrop;
  }

  if ( detectFlyAwayGesture() )
  {
    Serial.println( "TOF FlyAway gesture detected" );    
    recentGesture = TOFGesture::FlyAway;
  }

}

/*
  Hold your palm or finger over the sensor to indicate a Sleep gesture
*/

bool TOF::detectSleepGesture() 
{
  closeReadingsCount = 0;
  maxCount = 0;

  GestureData recentValues[ TOF_BLOCK_SIZE ];
  getMostRecentValues( recentValues, TOF_BLOCK_SIZE );

  //int lf = 0;

  for ( int i = 0; i < TOF_BLOCK_SIZE; i++ ) 
  {
    /*
    Serial.print( recentValues[i].distance );
    Serial.print( ", ");

    lf++;
    if ( lf > 7 )
    {
      Serial.println( " " );
      lf = 0;
    }*/

    // Check if the detected distance is within the 1-2 inch range
    if ( ( recentValues[i].distance > detectionThresholdLow ) && ( recentValues[i].distance < detectionThresholdHigh ) ) 
    {
      closeReadingsCount++;
    }

    if ( recentValues[i].distance > cancelHighRejection ) maxCount++;
  }

  /*
  Serial.print( "closeReadingsCount " );
  Serial.print( closeReadingsCount );
  Serial.print( " maxCount " );
  Serial.println( maxCount );
  */

  if ( maxCount > cancelRejectCount ) return false;

  // If the number of close readings exceeds the majority threshold, register a cancel gesture
  if ( closeReadingsCount > majorityThreshold )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool TOF::detectCircularGesture() 
{
  if ( millis() - circularTimeOut > circularDetectionDuration ) 
  {
    circularTimeOut = millis();

    accumulator[ 0 ] = false;
    accumulator[ 1 ] = false;
    accumulator[ 2 ] = false;
    accumulator[ 3 ] = false;

    return false;
  }

  if ( fingerTipInRange )
  {

    if ( ( fingerPosRow >= 0 ) && ( fingerPosRow < 4 ) )
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
      if ( ( fingerPosCol >= 0 ) && ( fingerPosCol < 4 ) )
      {
        // lower left
        accumulator[ 2 ] = true;
      }
      else
      {
        // lower right
        accumulator[ 3 ] = true;
      }
    }
  }

  Serial.print( "Circular gesture " );
  Serial.print( accumulator[0] );
  Serial.print( " " );
  Serial.print( accumulator[1] );
  Serial.print( " " );
  Serial.print( accumulator[2] );
  Serial.print( " " );
  Serial.println( accumulator[3] );

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
  if ( millis() - horizTimeOut > horizDetectionDuration ) 
  {
    horizTimeOut = millis();

    for ( int accx = 0; accx < 4; accx++ )
    {
      horizaccumulator[ accx ] = false;
    }
    
    return false;
  }

  if ( fingerTipInRange )
  {
    horizaccumulator[ fingerPosCol / 2 ] = true;
  }

  Serial.print( "Horizontal gesture " );
  Serial.print( horizaccumulator[0] );
  Serial.print( " " );
  Serial.print( horizaccumulator[1] );
  Serial.print( " " );
  Serial.print( horizaccumulator[2] );
  Serial.print( " " );
  Serial.println( horizaccumulator[3] );

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! horizaccumulator[ accx ] ) yahtzee = false;
  }

  if ( yahtzee )
  {
    for ( int accx = 0; accx < 4; accx++ )
    {
      horizaccumulator[ accx ] = false;
    }    
    return true;
  }

  return false;
}

bool TOF::detectVerticalGesture()
{
  if ( millis() - vertTimeOut > vertDetectionDuration ) 
  {
    vertTimeOut = millis();

    for ( int accx = 0; accx < 4; accx++ )
    {
      vertaccumulator[ accx ] = false;
    }
    
    return false;
  }

  if ( fingerTipInRange )
  {
    vertaccumulator[ fingerPosRow / 2 ] = true;
  }

  Serial.print( "Vertical gesture " );
  Serial.print( vertaccumulator[0] );
  Serial.print( " " );
  Serial.print( vertaccumulator[1] );
  Serial.print( " " );
  Serial.print( vertaccumulator[2] );
  Serial.print( " " );
  Serial.println( vertaccumulator[3] );

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! vertaccumulator[ accx ] ) yahtzee = false;
  }

  if ( yahtzee )
  {
    for ( int accx = 0; accx < 4; accx++ )
    {
      vertaccumulator[ accx ] = false;
    }    
    return true;
  }

  return false;
}

bool TOF::detectBombDropGesture()
{
 if ( millis() - bombTimeOut > bombDetectionDuration ) 
  {
    bombTimeOut = millis();

    for ( int accx = 0; accx < 4; accx++ )
    {
      bombaccumulator[ accx ] = false;
    }
    
    return false;
  }

  if ( fingerTipInRange )
  {
    bombaccumulator[ map( fingerDist, bombFlyDistLow, bombFlyDistHigh, 0, 3 ) ] = true;
  }

  Serial.print( "Bombdrop gesture " );
  Serial.print( bombaccumulator[0] );
  Serial.print( " " );
  Serial.print( bombaccumulator[1] );
  Serial.print( " " );
  Serial.print( bombaccumulator[2] );
  Serial.print( " " );
  Serial.println( bombaccumulator[3] );

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! bombaccumulator[ accx ] ) yahtzee = false;
  }

  if ( yahtzee )
  {
    for ( int accx = 0; accx < 4; accx++ )
    {
      bombaccumulator[ accx ] = false;
    }    
    return true;
  }

  return false;
}

bool TOF::detectFlyAwayGesture()
{
  if ( millis() - flyTimeOut > flyDetectionDuration ) 
  {
    flyTimeOut = millis();

    for ( int accx = 0; accx < 4; accx++ )
    {
      flyaccumulator[ accx ] = false;
    }
    
    return false;
  }

  if ( fingerTipInRange )
  {
    flyaccumulator[ map( fingerDist, bombFlyDistLow, bombFlyDistHigh, 0, 3 ) ] = true;
  }

  Serial.print( "FlyAway gesture " );
  Serial.print( flyaccumulator[0] );
  Serial.print( " " );
  Serial.print( flyaccumulator[1] );
  Serial.print( " " );
  Serial.print( flyaccumulator[2] );
  Serial.print( " " );
  Serial.println( flyaccumulator[3] );

  bool yahtzee = true;
  for ( int accx = 0; accx < 4; accx++ )
  {
    if ( ! flyaccumulator[ accx ] ) yahtzee = false;
  }

  if ( yahtzee )
  {
    for ( int accx = 0; accx < 4; accx++ )
    {
      flyaccumulator[ accx ] = false;
    }    
    return true;
  }

  return false;
}

bool TOF::detectFingerTip() 
{
  closeReadingsCount = 0;
  maxCount = 0;
  fingerTipInRange = false;

  GestureData recentValues[ TOF_BLOCK_SIZE ];
  getMostRecentValues( recentValues, TOF_BLOCK_SIZE );

  int myhappyindex = 0;
  int mystop = TOF_BLOCK_SIZE - 9;

  while ( myhappyindex < mystop )
  {
    int dist = recentValues[ myhappyindex ].distance;
    int dist2 = recentValues[ myhappyindex + 1 ].distance;
    int dist3 = recentValues[ myhappyindex + 8 ].distance;
    int dist4 = recentValues[ myhappyindex + 9 ].distance;

    /*
    Serial.print( myhappyindex );
    Serial.print( "\t" );
    Serial.print( dist );
    Serial.print( "\t" );
    Serial.print( dist2 );
    Serial.print( "\t" );
    Serial.print( dist3 );
    Serial.print( "\t" );
    Serial.println( dist4 );
    */

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

            fingerPosRow = myhappyindex / 8;

            if (fingerPosRow < 6) 
            {
              fingerPosRow = (fingerPosRow * 7) / 5;
            }
            else
            {
              // If original is 6 or 7, it maps directly to 7
              fingerPosRow = 7;
            }

            // Same scaling for columns

            fingerPosCol = myhappyindex % 8;

            if (fingerPosCol < 6) 
            {
              fingerPosCol = (fingerPosCol * 7) / 5;
            }
            else
            {
              // If originalColumn is 6 or 7, it maps directly to 7
              fingerPosCol = 7;
            }

            fingerDist = dist;
            fingerTipInRange = true;
            return true;
          }
        }
      }
    }
    myhappyindex++;
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

void TOF::loop()
{
  if ( ! started ) return;

  detectGestures();

/*
  if ( millis() - lastPollTime > 1000 ) 
  {
    lastPollTime = millis();

    showBubbles();
    printTOF();
  }
*/

}
