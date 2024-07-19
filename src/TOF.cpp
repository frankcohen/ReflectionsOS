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

#define diam 18
#define xdistance 30
#define ydistance 30
#define xspace 30
#define yspace 30
#define maxdist 50

#define GRID_ROWS 8
#define GRID_COLS 8

TOF::TOF() 
  : bufferIndex(0), currentBlockID(0), lastGestureTime(0), eyesFollowMode(false), bombDropDetected(false), flyAwayDetected(false) {
    initGestureBuffer();
}

void TOF::begin()
{ 
  started = false;
  lastPollTime = millis();
  nextCancelflag = false;
  
  // Create the task for sensor initialization
  xTaskCreate(
    TOF::sensorInitTaskWrapper,      // Function that implements the task
    "SensorInitTask",    // Text name for the task
    10000,               // Stack size in words, not bytes
    this,                // Parameter passed into the task
    1,                   // Priority at which the task is created
    &sensorInitTaskHandle // Handle to the created task
  );
}

void TOF::sensorInitTaskWrapper( void * parameter ) 
{
  // Cast the parameter to a VL53L5CX_Sensor pointer
  TOF *self = static_cast<TOF*>(parameter);
  // Call the instance task function
  self->sensorInitTask();
}

void TOF::sensorInitTask() 
{
  if ( sensor.begin( 0x29, Wire ) ) 
  {
    logger.info( F( "TOF sensor started" ) );

    cancelDetected = false;
    cancelGestureTimeout = millis();
    lastPollTime = millis();

    sensor.setResolution(8*8); //Enable all 64 pads

    sensor.setRangingFrequency(CAPTURE_RATE);

    imageResolution = sensor.getResolution();  //Query sensor for current resolution - either 4x4 or 8x8
    imageWidth = sqrt(imageResolution);           //Calculate printing width

    sensor.startRanging();

    started = true;
  }
  else
  {
    logger.info( F( "TOF sensor failed to initialize" ) );
  }

  gfx->begin();
  gfx->invertDisplay( true );
  gfx->fillScreen( COLOR_BACKGROUND );

  vTaskDelete(NULL); // Delete this task when done

  paceTime = millis();
  delayTime = millis();
}

void TOF::initGestureBuffer() 
{
  gestureBuffer = (GestureData*) malloc( TOF_BUFFER_SIZE * sizeof( GestureData ) );
  if (gestureBuffer == nullptr) {
    Serial.println("Failed to allocate memory for TOF gesture buffer");
    while (1); // Halt execution if allocation fails
  }
}

bool TOF::tofStatus()
{
  return started;
}

bool TOF::test()
{
  return started;  
}

bool TOF::cancelGestureDetected()
{
  if ( cancelDetected )
  {
    cancelDetected = false;
    nextCancelflag = true;
    return true;
  }
  else
  {
    return false;
  }  
}

int TOF::getReadingsCount()
{
  return closeReadingsCount;
}

int TOF::getNextGesture()
{
  if ( cancelDetected )
  {
    cancelDetected = false;
    return cancelled;
  }

  return 0;
}

/* Clears older gestures, allowing new ones to register */

void TOF::removeExpiredGestures()
{
  unsigned long timely = millis();

  if ( timely - cancelGestureTimeout > cancelDuration )
  {
    cancelDetected = false;
  }

  // Add additional timeouts here

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
  if ( millis() - delayTime < 180 ) return;
  delayTime = millis();

  if (sensor.isDataReady() == true) 
  {
    int distance;

    sensor.getRangingData( &measurementData );

    for (int i = 0; i < 8; i++) 
    {
      for (int j = 0; j < 8; j++) {
        distance = measurementData.distance_mm[ j + ( i * 8 ) ];

        int x = map(i, 0, 8, -50, 50); // Example mapping
        int y = map(j, 0, 8, -50, 50); // Example mapping

        gestureBuffer[bufferIndex] = {x, y, distance, currentBlockID};
        bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
      }
    }
    currentBlockID++;

    if (eyesFollowMode) 
    {
      GestureData recentValues[1];
      getMostRecentValues( recentValues, 1 );

      if (  recentValues[0].distance < 200 ) 
      {
        EyesFollow_Gesture( map(recentValues[0].x, -50, 50, 1, 8) );
      } 
      else 
      {
        eyesFollowMode = false;
        lastGestureTime = millis();
      }
    }
    else 
    {
      if (millis() - lastGestureTime > 3000) 
      {
        if ( detectCancelGesture() ) 
        {
          Cancel_Gesture();
        }
        else if ( false )  //detectCircularGesture()
        {
          Pounce_Gesture();
        } 
        else if ( false )  // detectLinearGesture(0, 0) 
        { 
          if (gestureBuffer[0].x > 0) 
          { // Replace with proper condition
            eyesFollowMode = true;
          } 
          else 
          {
            Parallax_Gesture();
          }
        }
        else if ( false )  //detectVerticalGesture() 
        {
          if (gestureBuffer[0].y > 0) 
          {
            Shaken_Gesture();
          }
          else
          {
            Chastise_Gesture();
          }
        } 
        else if ( false )  //detectBombDropGesture() 
        {
          BombDrop_Gesture();
        } 
        else if ( false )  //detectFlyAwayGesture() 
        {
          FlyAway_Gesture();
        } 
        else if ( false ) // detectFingerTip()
        {
          Serial.println("Finger tip detected.");
        }
        lastGestureTime = millis();
      }
    }
  }
}

/*
  Hold your palm or finger over the sensor to indicate a Cancel gesture
  Used often to start deep sleep
*/

bool TOF::detectCancelGesture() 
{
  closeReadingsCount = 0;

  GestureData recentValues[ TOF_BLOCK_SIZE ];
  getMostRecentValues( recentValues, TOF_BLOCK_SIZE );

  int lf = 0;

  for ( int i = 0; i < TOF_BLOCK_SIZE; i++ ) 
  {
    Serial.print( recentValues[i].distance );
    Serial.print( ", ");

    lf++;
    if ( lf > 7 )
    {
      Serial.println( " " );
      lf = 0;
    }

    // Check if the detected distance is within the 1-2 inch range
    if ( ( recentValues[i].distance > detectionThresholdLow ) && ( recentValues[i].distance < detectionThresholdHigh ) ) 
    {
      closeReadingsCount++;
    }
  }

  Serial.print( "closeReadingsCount " );
  Serial.println( closeReadingsCount );
  
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
  const int accumulatorSize = 50;
  int accumulator[accumulatorSize][accumulatorSize] = {0};

  for (int i = 0; i < BUFFER_SIZE; i++) 
  {
    int x = gestureBuffer[i].x;
    int y = gestureBuffer[i].y;

    for (int r = 1; r < 20; r++)     // Adjust MAX_RADIUS as needed
    {
      for (int theta = 0; theta < 360; theta++) 
      {
        int a = x - r * cos(theta * PI / 180);
        int b = y - r * sin(theta * PI / 180);
        if (a >= 0 && a < accumulatorSize && b >= 0 && b < accumulatorSize) 
        {
          accumulator[a][b]++;
        }
      }
    }
  }

  for (int a = 0; a < accumulatorSize; a++) {
      for (int b = 0; b < accumulatorSize; b++) {
          if (accumulator[a][b] > 5) { // Adjust centerThreshold as needed
              return true;
          }
      }
  }

  return false;
}

bool TOF::detectLinearGesture(int x, int y) {
    if (bufferIndex < minPointsForLine) return false;

    float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        sumX += gestureBuffer[i].x;
        sumY += gestureBuffer[i].y;
        sumXY += gestureBuffer[i].x * gestureBuffer[i].y;
        sumX2 += gestureBuffer[i].x * gestureBuffer[i].x;
    }
  
    float n = BUFFER_SIZE;
    float slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
  
    float intercept = (sumY - slope * sumX) / n;
    float errorSum = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        float predictedY = slope * gestureBuffer[i].x + intercept;
        errorSum += abs(gestureBuffer[i].y - predictedY);
    }
  
    float averageError = errorSum / BUFFER_SIZE;
    return averageError < linearThreshold;
}

bool TOF::detectVerticalGesture() {
    if (bufferIndex < minPointsForLine) return false;

    float sumX = 0, sumY = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        sumX += gestureBuffer[i].x;
        sumY += gestureBuffer[i].y;
    }
    float avgX = sumX / BUFFER_SIZE;
    float avgY = sumY / BUFFER_SIZE;

    float varianceX = 0, varianceY = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        varianceX += pow(gestureBuffer[i].x - avgX, 2);
        varianceY += pow(gestureBuffer[i].y - avgY, 2);
    }
    varianceX /= BUFFER_SIZE;
    varianceY /= BUFFER_SIZE;

    return (varianceX < linearThreshold && varianceY > linearThreshold);
}

bool TOF::detectBombDropGesture() {
    const int highThreshold = 1524;
    const int lowThreshold = 25;
  
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (gestureBuffer[i].distance > highThreshold) {
            bombDropDetected = true;
        }
        if (bombDropDetected && gestureBuffer[i].distance < lowThreshold) {
            bombDropDetected = false;
            return true;
        }
    }
    return false;
}

bool TOF::detectFlyAwayGesture() {
    const int highThreshold = 1524;
    const int lowThreshold = 25;
  
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (gestureBuffer[i].distance < lowThreshold) {
            flyAwayDetected = true;
        }
        if (flyAwayDetected && gestureBuffer[i].distance > highThreshold) {
            flyAwayDetected = false;
            return true;
        }
    }
    return false;
}

bool TOF::detectFingerTip() {
    int minDistance = 10000; // Initialize with a large value
    int minIndex = -1;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (gestureBuffer[i].distance < minDistance) {
            minDistance = gestureBuffer[i].distance;
            minIndex = i;
        }
    }

    if (minIndex != -1 && minDistance < 50) { // Adjust threshold as needed
        Serial.print("Finger tip detected at (");
        Serial.print(gestureBuffer[minIndex].x);
        Serial.print(", ");
        Serial.print(gestureBuffer[minIndex].y);
        Serial.println(")");
        return true;
    }
    return false;
}

// Placeholder gesture handler functions
void TOF::Cancel_Gesture() {
    Serial.println("Cancel Gesture detected.");
}

void TOF::Pounce_Gesture() {
    Serial.println("Pounce Gesture detected.");
}

void TOF::EyesFollow_Gesture(int position) {
    Serial.print("EyesFollow Gesture detected. Position: ");
    Serial.println(position);
}

void TOF::Parallax_Gesture() {
    Serial.println("Parallax Gesture detected.");
}

void TOF::Shaken_Gesture() {
    Serial.println("Shaken Gesture detected.");
}

void TOF::Chastise_Gesture() {
    Serial.println("Chastise Gesture detected.");
}

void TOF::BombDrop_Gesture() {
    Serial.println("BombDrop Gesture detected.");
}

void TOF::FlyAway_Gesture() {
    Serial.println("FlyAway Gesture detected.");
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

          if ( dist > maxdist )
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
        Serial.println();
      }
      Serial.println();
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
      float dist = measurementData.distance_mm[ x + ( y * GRID_COLS ) ];

      if ( dist > maxdist ) dist = maxdist;
      
      int diam2 = (int) ( ( (float) diam ) * ( dist / maxdist ) );
      
      gfx -> fillCircle( xspace + ( y * ydistance ), xspace + ( x * xdistance ), diam, COLOR_BACKGROUND);
      
      if ( dist > 0 )
      {
        gfx -> drawCircle( yspace + ( y * ydistance ), xspace + ( x * xdistance ), diam2, COLOR_RING);
      }
    }
  }
}

void TOF::loop()
{
  if ( ! started ) return;

  detectGestures();

  if ( millis() - lastPollTime > 500 ) 
  {
    lastPollTime = millis();

    showBubbles();
    //printTOF();
  }

}
