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

  // Allocate memory for all sets in one go
  buffer = (int16_t*) malloc( ( NUM_SETS + 4 ) * SET_SIZE * sizeof(int16_t) );
  
  if (buffer == NULL) {
    Serial.println( F( "TOF failed to allocate memory for TOF gesture buffer" ) );
    while (1); // Stop execution if memory allocation fails
  }

  // Initialize the buffer (optional, e.g., set all values to 0)
  memset( buffer, 0, NUM_SETS * SET_SIZE * sizeof(int16_t) );

  currentSetIndex = 0;

  fingerTipInRange = false;
  fingerPosRow = 0;
  fingerPosCol = 0;

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

  myMef = "";
  myMef2 = "";

  for (int i = 0; i<8; i++ )
  {
    previousHorizPositions[i] = -1;
  }

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
  if ( recentGesture != TOFGesture::None ) return;

  fingerTipInRange = detectFingerTip( currentSetIndex );

  if ( detectSleepGesture() )
  {
    sleepCount++;
    recentGesture = TOFGesture::Sleep;
    return;
  }

  // Need the finger tip for the rest of these to work

  if ( ! fingerTipInRange )
  {
    return;
  };

/*
  if ( detectVerticalGesture() )
  {
    recentGesture = TOFGesture::Vertical;
    myMef2 += " (TOF vert)";
    return;
  }
*/

  if ( detectHorizontalGesture() )
  {
    recentGesture = TOFGesture::Horizontal;
    myMef += " (TOF horiz)";
    return;
  }

  if ( detectCircularGesture() )
  {
    recentGesture = TOFGesture::Circular;
    myMef2 = " (TOF circular)";
    return;
  }

  if ( detectBombDropGesture() )
  {
    recentGesture = TOFGesture::BombDrop;
    myMef2 = " (TOF bomb drop)";
    return;
  }

  if ( detectFlyAwayGesture() )
  {
    recentGesture = TOFGesture::FlyAway;
    myMef2 = " (TOF fly away)";
    return;
  }

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

  float scount = 0;
  int16_t mval = 0;

  for ( int mia = 0; mia < SET_SIZE; mia++ )
  {
    mval = recentSet[ mia ];
    if ( ( mval > sleepLowFilter ) && ( mval < sleepHighFilter ) ) scount++;
  }
  
  /*
  myMef = "sleep: ";
  myMef += scount;
  myMef += ", ";
  myMef += ( scount / SET_SIZE );
  myMef += ", ";

  for ( int j = 0; j < 64; j++ )
  {
    myMef += recentSet[ j ];
    myMef += " ";
  }
  myMef += "@";
  */

  if ( ( scount / SET_SIZE ) > sleepPercentage )
  {
    return true;
  }

  return false;
}

bool TOF::detectCircularGesture() 
{
  bool rowDetected[GRID_ROWS] = {false};
  bool columnDetected[ GRID_COLS ] = {false};

  // Iterate through the last 5 calls to detectFingerTip and update the column and row tracking
  for ( int i = 0; i < 5; i++ ) 
  {
    // Call detectFingerTip() and store the result in fingerPosCol and fingerPosRow if a finger is detected
    if ( detectFingerTip( currentSetIndex - i ) ) 
    {
      if ( ( fingerPosRow >= 0 ) && ( fingerPosRow < GRID_ROWS ) ) 
      {
        rowDetected[ fingerPosRow ] = true;
      }

      if ( ( fingerPosCol >= 0 ) && ( fingerPosCol < GRID_COLS ) ) 
      {
        columnDetected[ fingerPosCol ] = true;
      }
    }
  }

/*
  myMef = "Circular: rowDetected ";
  for ( int i = 0; i<64; i++)
  {
    myMef += rowDetected[i];
    myMef += " ";
  }
  myMef += "columnDetected ";
  for ( int i = 0; i<64; i++)
  {
    myMef += columnDetected[i];
    myMef += " ";
  }


*/

  // Check for contiguous rows and columns forming a 3x3 block
  for ( int row = 0; row < GRID_ROWS - 2; row++ ) 
  {
    for ( int col = 0; col < GRID_COLS - 2; col++ ) 
    {
      // Check if a 3x3 block is formed by contiguous rows and columns
      if ( rowDetected[row] && rowDetected[row + 1] && rowDetected[row + 2] && 
          columnDetected[col] && columnDetected[col + 1] && columnDetected[col + 2] ) 
      {
        // If a 3x3 block is detected, return true
        return true;
      }
    }
  }

  return false;
}

bool TOF::detectHorizontalGesture() 
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

    int16_t* newFrame = buffer + (indexNew * SET_SIZE);
    int16_t* oldFrame = buffer + (indexOld * SET_SIZE);

    myMef2 = "";

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
  
  if ( ( leftMovement > 50 ) || ( rightMovement > 50 ) )
  {
    myMef = "Circular ";
    myMef += leftMovement;    
    myMef += " ";
    myMef += rightMovement;
    return true;
  } 

  if ( ( leftMovement > 2 ) || ( rightMovement > 2 ) )
  {
    if ( leftMovement > rightMovement ) 
    {
      myMef = "Left ";
      myMef += leftMovement;    
      myMef += " ";
      myMef += rightMovement;
      return true;
    } 
    
    else if ( rightMovement > leftMovement) 
    {
      myMef = "Right ";
      myMef += leftMovement;    
      myMef += " ";
      myMef += rightMovement;
      return true;
    }
  }

  if ( ( upMovement > 2 ) || ( downMovement > 2 ) )
  {
    if ( upMovement > downMovement ) 
    {
      myMef = "Up ";
      myMef += upMovement;    
      myMef += " ";
      myMef += downMovement;
      return true;
    } 
    
    else if ( downMovement > upMovement ) 
    {
      myMef = "Down ";
      myMef += upMovement;    
      myMef += " ";
      myMef += downMovement;
      return true;
    } 
  }

  myMef = "Nope ";
  myMef += leftMovement;    
  myMef += " ";
  myMef += rightMovement;
  myMef += " ";
  myMef += upMovement;
  myMef += " ";
  myMef += downMovement;

  return false;
}


bool TOF::detectHorizontalGesture4() 
{
  myMef = "h: ";
  myMef2 = ">";

  int hcntleft = 0;
  int hcntright = 0;

  for (int i = 0; i < ( movementFrames - 1); i++) 
  {
    // Calculate indices for the newer and older frames in the pair.
    int indexNew = (currentSetIndex - 1 - i + NUM_SETS) % NUM_SETS;
    int indexOld = (currentSetIndex - 2 - i + NUM_SETS) % NUM_SETS;

    int16_t* newFrame = buffer + (indexNew * SET_SIZE);
    int16_t* oldFrame = buffer + (indexOld * SET_SIZE);

  myMef2 += "frame ";
  myMef2 += i;

  myMef2 += " new: ";
  for ( int k = 0; k<64; k++ )
  {
    myMef2 += newFrame[ k ];
    myMef2 += " ";
  }
  myMef2 += ", old: ";
  for ( int k = 0; k<64; k++ )
  {
    myMef2 += oldFrame[ k ];
    myMef2 += " ";
  }

    for (int j = 0; j < 7; j++ )
    {
      int16_t new1 = newFrame[ j + 1 + ( 8 * 4 ) ];
      int16_t old1 = oldFrame[ j + 0 + ( 8 * 4 ) ];

      if ( 
         ( ( new1 > movementLow ) && ( new1 < movementHigh ) ) &&
         ( ( old1 > movementLow ) && ( old1 < movementHigh ) ) &&
         ( abs( new1 - old1 ) <= 4 ) 
        )
      { 
        hcntleft++;
      }

      int16_t new2 = newFrame[ j + 0 + ( 8 * 4 ) ];
      int16_t old2 = oldFrame[ j + 1 + ( 8 * 4 ) ];

      if (
         ( ( new2 > movementLow ) && ( new2 < movementHigh ) ) &&
         ( ( old2 > movementLow ) && ( old2 < movementHigh ) ) &&
         ( abs( new2 - old2 ) <= 4 ) 
        )
      { 
        hcntright++;
      }      

      myMef += new1;
      myMef += ".";
      myMef += old1;
      myMef += ".";
      myMef += new2;
      myMef += ".";
      myMef += old2;
      myMef += ", ";
     



    }
  }

  myMef += hcntleft;
  myMef += ", ";
  myMef += hcntright;
  myMef += ", ";
  myMef += currentSetIndex;
  myMef += ", ";

  return false;
}


bool TOF::detectHorizontalGesture3() 
{
  int total_dx = 0;
  int total_dy = 0;
  int count = 0;

  for (int i = 0; i < FRAMES_TO_ANALYZE - 1; i++) 
  {
    // Calculate indices for the newer and older frames in the pair.
    int indexNew = (currentSetIndex - 1 - i + NUM_SETS) % NUM_SETS;
    int indexOld = (currentSetIndex - 2 - i + NUM_SETS) % NUM_SETS;

    int16_t* newerFrame = buffer + (indexNew * SET_SIZE);
    int16_t* olderFrame = buffer + (indexOld * SET_SIZE);
    
    int dx = 0, dy = 0;

    determineMovementBetweenFrames(olderFrame, newerFrame, dx, dy);

    total_dx += dx;
    total_dy += dy;
    count++;
  }

  int avg_dx = 0;
  int avg_dy = 0;
  
  if ( count > 0 )
  {
    avg_dx = total_dx / count;
    avg_dy = total_dy / count;

    if ( ( avg_dx != 0 ) && ( avg_dy != 0 ) )
    {
      myMef = "Horiz: ";
      myMef += avg_dx;
      myMef += " ";
      myMef += avg_dy;
    }
  }

  return false;
}

// Compute cross-correlation between two 8x8 frames for a given shift (dx, dy).
// We treat the 1D array as a 2D array using index = row*8 + col.
float TOF::computeCorrelation( int16_t *frame1, int16_t *frame2, int dx, int dy) 
{
  float sum = 0;
  int count = 0;
  const int SIZE = 8;
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      int shifted_i = i + dy;
      int shifted_j = j + dx;
      // Only compute for overlapping parts.
      if (shifted_i >= 0 && shifted_i < SIZE && shifted_j >= 0 && shifted_j < SIZE) {
        sum += frame1[i * SIZE + j] * frame2[shifted_i * SIZE + shifted_j];
        count++;
      }
    }
  }
  return (count > 0) ? sum / count : 0;
}

// Determines the movement (dx, dy) between an older frame and a newer frame
// by testing small shifts and choosing the one with maximum correlation.
void TOF::determineMovementBetweenFrames(int16_t * olderFrame, int16_t * newerFrame, int &best_dx, int &best_dy) 
{
  // Check if buffers are valid pointers
  if ( olderFrame == NULL )
  {
    Serial.println("TOF determineMovementBetweenFrames: olderFrame is null");
    return;
  }

  if ( newerFrame == NULL ) 
  {
    Serial.println("TOF determineMovementBetweenFrames: newerFrame is null");
    return;
  }

  float maxCorr = -1e6;
  best_dx = 0;
  best_dy = 0;
  
  // Try shifts in the range [-3, 3]. Adjust based on expected movement speed.
  for (int dx = -3; dx <= 3; dx++) {
    for (int dy = -3; dy <= 3; dy++) {
      float corr = computeCorrelation(olderFrame, newerFrame, dx, dy);
      if (corr > maxCorr) {
        maxCorr = corr;
        best_dx = dx;
        best_dy = dy;
      }
    }
  }
}


bool TOF::detectHorizontalGesture2() 
{  
  // Array to store the positions of detected objects for each row in the 10 most recent frames
  int currentPositions[8] = {-1};   // Stores column positions (or -1 if no object detected)
  bool movementDetected = false;    // To keep track if movement was detected in one direction
  bool lastDirectionRight = false;  // Track last movement direction (true right, false = left)
  int movementDirectionCounter = 0;  // Counter to track consecutive frames with the same direction
  bool newMovementDetected = false;

    myMef = "Horiz: ";
    for (int j = 0; j< 8; j++ )
    {
      myMef += previousHorizPositions[ j ];
      myMef += " ";
    }

  for (int frame = 0; frame < 10; frame++) 
  {
    int recentSetIndex = ( currentSetIndex - 1 - frame + NUM_SETS ) % NUM_SETS;
    int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

    // Check each row for object positions in the current frame
    for (int row = 0; row < 8; row++) 
    {
      for (int col = 0; col < 8; col++) 
      {
        int16_t distance = recentSet[ ( row * 8 ) + col ];
            
        if ( ( distance >= fingerDetectionThresholdLow ) && ( distance <= fingerDetectionThresholdHigh ) ) 
        {
          currentPositions[row] = col;  // Object detected at this column
          break;  // Stop after finding the first object in the row
        }
      }
    }

    myMef += ", c: ";
    for (int j = 0; j < 8; j++ )
    {
      myMef += currentPositions[ j ];
      myMef += " ";
    }


    if (frame > 0) 
    {
      for (int row = 0; row < 8; row++) 
      {
        if ( ( currentPositions[row] != -1 ) && ( previousHorizPositions[row] != -1 ) ) 
        {
          if (currentPositions[row] > ( previousHorizPositions[row] ) ) 
          {
            // Moving to the right
            if (!movementDetected || ! lastDirectionRight) 
            {
              movementDetected = true;
              lastDirectionRight = true;
              movementDirectionCounter++;
            }
          }
          else if (currentPositions[row] < ( previousHorizPositions[row] ) ) 
          {
            // Moving to the left
            if (!movementDetected || lastDirectionRight) 
            {
              movementDetected = true;
              lastDirectionRight = false;
              movementDirectionCounter++;
            }
          }
        }
      }

      // Store the current positions for comparison in the next frame
      memcpy(previousHorizPositions, currentPositions, sizeof(previousHorizPositions));

      if (movementDirectionCounter >= 4) 
      {
        newMovementDetected = true;
      }

      // Reset the counter if the direction has changed
      if ( ( lastDirectionRight && ( movementDirectionCounter > 0 ) ) || ( !lastDirectionRight && ( movementDirectionCounter > 0 ) ) ) 
      {
        movementDirectionCounter = 0;
        newMovementDetected = false;
      }

      myMef += " : ";
      myMef += movementDetected;
      myMef += " ";
      myMef += newMovementDetected;
      myMef += " ";
      myMef += lastDirectionRight;
      myMef += " ";
      myMef += movementDirectionCounter;
      myMef += ", ";

    }
  }

  if ( lastDirectionRight )
  {
    myMef += " right,";
  }
  else
  {
    myMef += " left,";
  }
  
  return newMovementDetected;
}

bool TOF::detectVerticalGesture()
{  
  // Array to store the positions of detected objects for each column in the 10 most recent frames
  int currentPositions[8] = {-1};   // Stores row positions (or -1 if no object detected)
  bool movementDetected = false;    // To keep track if movement was detected in one direction
  bool lastDirectionUp = false;  // Track last movement direction (true right, false = left)
  int movementDirectionCounter = 0;  // Counter to track consecutive frames with the same direction
  bool newVertMovementDetected = false;

  myMef2 = "Vertical: ";

  for (int j = 0; j< 8; j++ )
  {
    myMef2 += previousVertPositions[ j ];
    myMef2 += " ";
  }

  for (int frame = 0; frame < 10; frame++) 
  {
    int recentSetIndex = ( currentSetIndex - 1 - frame + NUM_SETS ) % NUM_SETS;
    int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );
    
    // Check each row for object positions in the current frame
    for (int col = 0; col < 8; col++) 
    {
      for (int row = 0; row < 8; row++) 
      {
        int16_t distance = recentSet[ ( row * 8 ) + col];  // Get distance value for this row, column
            
        if ( ( distance >= fingerDetectionThresholdLow ) && ( distance <= fingerDetectionThresholdHigh ) ) 
        {
          currentPositions[col] = row;  // Object detected at this column
          break;  // Stop after finding the first object in the row
        }
      }
    }

    if (frame > 0) 
    {
      for (int col = 0; col < 8; col++) 
      {
        if ( ( currentPositions[ col ] != -1 ) && previousVertPositions[ col ] != -1) 
        {
          if (currentPositions[col] > previousVertPositions[col] ) 
          {
            // Moving down
            if ( ! movementDetected || ! lastDirectionUp) 
            {
              movementDetected = true;
              lastDirectionUp = false;
              movementDirectionCounter++;
            }
          }
          else if (currentPositions[ col ] < previousVertPositions[ col ] ) 
          {
            // Moving to the left
            if (!movementDetected || lastDirectionUp) 
            {
              movementDetected = true;
              lastDirectionUp = true;
              movementDirectionCounter++;
            }
          }
        }
      }

      if (movementDirectionCounter >= 4) 
      {
        newVertMovementDetected = true;
      }
        
      // Store the current positions for comparison in the next frame
      memcpy(previousVertPositions, currentPositions, sizeof(previousVertPositions));

      // Reset the counter if the direction has changed
      if ((lastDirectionUp && movementDirectionCounter > 0) || (!lastDirectionUp && movementDirectionCounter > 0)) 
      {
        movementDirectionCounter = 0;
      }

    }
  }

  if ( lastDirectionUp )
  {
    myMef2 += " up,";
  }
  else
  {
    myMef2 += " down,";
  }
  
  return newVertMovementDetected;
}

bool TOF::detectBombDropGesture()
{
  int lastDistances[5] = {0, 0, 0, 0, 0};  // Initialize with default values

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( currentSetIndex - 1 + NUM_SETS ) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  // Iterate through the last 5 calls to detectFingerTip and update the column tracking
  for ( int i = 0; i < 5; i++ ) 
  {
    // Call detectFingerTip() and store the result in fingerPosCol if a finger is detected
    if ( detectFingerTip( currentSetIndex - i ) ) 
    {
      float dist = recentSet[ ( fingerPosCol * GRID_COLS ) + fingerPosRow ];
      if ( dist > 0 )
      {
        lastDistances[ i ] = dist;
      }
    }
  }

  for (int i = 0; i < 4; i++) 
  {
    if ( lastDistances[i] <= lastDistances[i + 1] ) 
    {
      return false;  // If any value is greater or equal to the next, return false
    }

  }

  return true;  // All values are descending
}

bool TOF::detectFlyAwayGesture()
{
  int lastDistances[5] = {0, 0, 0, 0, 0};  // Initialize with default values

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( currentSetIndex - 1 + NUM_SETS ) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  // Iterate through the last 5 calls to detectFingerTip and update the column tracking
  for ( int i = 0; i < 5; i++ ) 
  {
    // Call detectFingerTip() and store the result in fingerPosCol if a finger is detected
    if ( detectFingerTip( currentSetIndex - i ) ) 
    {
      float dist = recentSet[ ( fingerPosCol * GRID_COLS ) + fingerPosRow ];
      if ( dist > 0 )
      {
        lastDistances[ i ] = dist;
      }
    }
  }

  for (int i = 0; i < 4; i++) 
  {
    if ( lastDistances[i] >= lastDistances[i + 1] ) 
    {
      return false;  // If any value is greater or equal to the next, return false
    }

  }
  
  return true;  // All values are descending
}

// Finger tip coordinates used by Pounce, Eyes, Parallax, Horiz, Vert

bool TOF::detectFingerTip( int setnum ) 
{
  if ( setnum > NUM_SETS ) return false;

  fingerTipInRange = false;
  fingerPosRow = 0;
  fingerPosCol = 0;

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = ( setnum - 1 + NUM_SETS) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

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

    /*
    myMef = "dist = ";
    myMef += fingerDist;
    myMef += ", ";
    myMef += fingerPosRow;
    myMef += ", ";
    myMef += fingerPosCol;
    myMef += ", ";
    myMef += ( fingerPosCol * GRID_COLS ) + fingerPosRow;
    myMef += ", ";
    myMef += minX;
    myMef += ", ";
    myMef += maxX;
    myMef += ", ";
    myMef += minY;
    myMef += ", ";
    myMef += maxY;
    myMef += ", ";

    for ( int j = 0; j < 64; j++ )
    {
      myMef += recentSet[ j ];
      myMef += ", ";
    }
    */

    // Color the finger center point

    //int spotdiam = ( ( fingerDist - fingerDetectionThresholdLow ) / ( fingerDetectionThresholdHigh - fingerDetectionThresholdLow ) ) * tofdiam;

    int16_t xloc = xspace + ( fingerPosCol * xdistance );
    int16_t yloc = yspace + ( fingerPosRow * ydistance );
    gfx -> fillCircle( xloc, yloc, tofdiam, COLOR_BLUE);

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

  // BombDrop gesture accumulators
  mef += "Bomb: [";
  for (int i = 0; i < 4; i++) {
      mef += bombaccumulator[i] ? "1" : "0";
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
  String myMefa = myMef;
  String myMef = "";
  return myMefa;
}

String TOF::getMef2()
{
  String myMefa = myMef2;
  String myMef2 = "";
  return myMefa;
}

void TOF::flipAndRotateArray(int16_t* dest, int width, int height) 
{
/*
  // fcohen@starlingwatch.com I have no idea why the sensor data needs
  // to be flipped horizontally and then rotated 90 degreeS BUT does
  // not also need to be flipped vertically. I decided to move on.

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

void TOF::acquireDataToBuffer()
{
  if ( ! sensor.getRangingData( &measurementData ) )
  {
    SF_VL53L5CX_ERROR_TYPE errorCode = sensor.lastError.lastErrorCode;
    Serial.println( "TOF sensor error" );
    Serial.println( (int) errorCode );
    return;
  }
  
  // Check if buffer and measurementData are valid
  if ( buffer == NULL ) 
  {
    Serial.println("TOF Error: buffer is null");
    return;
  }

  if ( measurementData.distance_mm == NULL ) 
  {
    Serial.println("TOF Error: measurementData.distance_mm is null");
    return;
  }

  // Ensure currentSetIndex is within bounds for the buffer
  if ( currentSetIndex >= 100 ) 
  {
    Serial.println("Error: currentSetIndex is out of bounds");
    return;
  }

  // Get the pointer to the current set's storage in the buffer
  int16_t* dest = buffer + ( currentSetIndex * SET_SIZE );

  // Copy the measurement data to the buffer
  memcpy( dest, measurementData.distance_mm, SET_SIZE * sizeof( int16_t ) );

  flipAndRotateArray( dest, 8, 8 );

  // Move to the next set, wrapping around
  currentSetIndex = ( currentSetIndex + 1 ) % NUM_SETS;

  // Get the pointer to the most recent set's storage in the buffer
  int recentSetIndex = (currentSetIndex - 1 + NUM_SETS) % NUM_SETS;
  int16_t* recentSet = buffer + ( recentSetIndex * SET_SIZE );

  // Filter out the invalid readings

  for ( int mia = 0; mia < SET_SIZE; mia++ )
  {
    if ( ( recentSet[ mia ] < closefilter ) || ( recentSet[ mia ] > farfilter ) ) 
    {
      recentSet[ mia ] = 0;
    }
  }

  // Show bubbles on display

  /*
    myMef2 = "Bubbles: ";
    for ( int j = 0; j < 64; j++ )
    {
      myMef2 += recentSet[ j ];
      myMef2 += ", ";
    }
    myMef2 += "\n#";
  */

  for ( int y = 0; y < 8; y++ )
  {
    for ( int x = 0; x < 8; x++ )
    {
      int16_t xloc = xspace + ( x * xdistance );
      int16_t yloc = yspace + ( y * ydistance );
      int16_t offset = (y * 8) + x;
      int16_t spot = recentSet[ offset ];

      /*
      myMef += offset;
      myMef += " ";
      myMef += x;
      myMef += " ";
      myMef += y;
      myMef += " ";
      myMef += xloc;
      myMef += " ";
      myMef += yloc;
      myMef += " ";
      myMef += spot;
      */

      gfx -> fillCircle( xloc, yloc, tofdiam, COLOR_BACKGROUND);

      if ( ( spot > bubbleLow ) && ( spot < bubbleHigh ) ) 
      {
        float spot1 = spot - bubbleLow;
        float spot2 = bubbleHigh - bubbleLow ;
        float spot3 = spot1 / spot2;
        int16_t spotdiam = round( spot3 * tofdiam );

        //myMef += "*";
        //myMef += spotdiam;

        if ( spotdiam > 0 )
        {
          gfx -> drawCircle( xloc, yloc, spotdiam, COLOR_RING);
        }
      }

      //myMef += ", ";
    }
    //myMef += "\n\n";
  }
    //myMef += "\n\n\n";
}

void TOF::loop()
{
  if ( ! started ) return;

  if ( millis() - previousMillis > 250 ) 
  {
    previousMillis = millis();

    acquireDataToBuffer();    // And show bubbles

    //detectGestures();


    if ( detectHorizontalGesture() )
    {
      recentGesture = TOFGesture::Horizontal;
      //myMef = " (TOF horiz)";
    }




  }

  if ( millis() - debouncetime > 1000 ) 
  {
    if ( recentGesture != TOFGesture::None )
    {
      recentGesture = TOFGesture::None;
      debouncetime = millis();
    }
  }

}
