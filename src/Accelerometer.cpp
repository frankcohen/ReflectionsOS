/*
  Reflections, distributed entertainment device

  Recognizes gestures by moving the Reflections board 
  (LIS3DHTR 3-Axis Accelerometer)

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Supports multiple gestures and stores them on the SD/NAND
  Each gesture is 1.5 seconds, length is adjustable in the code

  Depends on:
  https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library

  Modified from Federico Terzi experiment at
  https://github.com/CieloStrive/GESTURE-RECOGNITION-DYNAMIC-TIME-WARPING/tree/master

  Future:
  Add a gesture pruning service to remove lowest rated gestures

*/

#include "Accelerometer.h"

extern Storage storage;   // Defined in ReflectionsOfFrank.ino
extern Haptic haptic;
extern Utils utils;
extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

Accelerometer::Accelerometer() : myIMU(I2C_MODE, 0x18) {}

void Accelerometer::begin()
{ 
  delay(100);

  myIMU.settings.adcEnabled = 1;
  myIMU.settings.tempEnabled = 1;
  myIMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 16;      //Max G force readable.  Can be: 2, 4, 8, 16
  myIMU.settings.xAccelEnabled = 1;
  myIMU.settings.yAccelEnabled = 1;
  myIMU.settings.zAccelEnabled = 1;

  myIMU.begin();

  uint8_t dataToWrite = 0;

  // Configure interrupts
  // LIS3DH_INT1_CFG   
  dataToWrite = 0x08; // Enable Y high interrupt
  myIMU.writeRegister(LIS3DH_INT1_CFG, dataToWrite);

  // LIS3DH_INT1_THS   
  dataToWrite = 0x10; // Set threshold (adjust as needed)
  myIMU.writeRegister(LIS3DH_INT1_THS, dataToWrite);
  
  // LIS3DH_INT1_DURATION  
  dataToWrite = 0x01; // Set duration (1 * 1/50 s = 20ms)
  myIMU.writeRegister(LIS3DH_INT1_DURATION, dataToWrite);

  // Configure click detection (optional)
  // LIS3DH_CLICK_CFG   
  dataToWrite = 0x15; // Enable click detection on X, Y, and Z axes
  myIMU.writeRegister(LIS3DH_CLICK_CFG, dataToWrite);

  // LIS3DH_CLICK_SRC
  dataToWrite = 0x07; // Enable single clicks on X, Y, and Z axes
  myIMU.writeRegister(LIS3DH_CLICK_SRC, dataToWrite);

  // LIS3DH_CLICK_THS   
  dataToWrite = 0x0A; // Set click threshold
  myIMU.writeRegister(LIS3DH_CLICK_THS, dataToWrite);

  // LIS3DH_TIME_LIMIT  
  dataToWrite = 0x08; // Set time limit for click detection
  myIMU.writeRegister(LIS3DH_TIME_LIMIT, dataToWrite);

  // LIS3DH_TIME_LATENCY
  dataToWrite = 0x08; // Set time latency for click detection
  myIMU.writeRegister(LIS3DH_TIME_LATENCY, dataToWrite);

  // LIS3DH_TIME_WINDOW 
  dataToWrite = 0x10; // Set time window for double-click detection
  myIMU.writeRegister(LIS3DH_TIME_WINDOW, dataToWrite);
  
  // LIS3DH_CTRL_REG5
  myIMU.readRegister(&dataToWrite, LIS3DH_CTRL_REG5);
  dataToWrite &= 0xF3; // Clear bits of interest
  dataToWrite |= 0x08; // Latch interrupt
  myIMU.writeRegister(LIS3DH_CTRL_REG5, dataToWrite);

  // LIS3DH_CTRL_REG3
  dataToWrite = 0x60; // Enable AOI1 and AOI2 events on INT1 pin
  myIMU.writeRegister(LIS3DH_CTRL_REG3, dataToWrite);

  // LIS3DH_CTRL_REG6
  dataToWrite = 0x80; // Enable click interrupt on INT2 pin
  myIMU.writeRegister(LIS3DH_CTRL_REG6, dataToWrite);

  movetimer = millis();
  detecttimer = millis();
  firsttime = true;
  olddtw = 0;
  recordi = 0;
  movetimer = millis();
  firstnotice = false;
  
  gesturenumber = 0;
  gesturecount = 0;
  cmpcount = 0; 
  gesturesloaded = false;
  currentreading = false;
  gesturestart = false;
  recordingTemplate = false;
  typecounter = 0;

  nx0 = 1, nx1 = 1, nx2 = 1, nx3 = 1;
  ny0 = 1, ny1 = 1, ny2 = 1, ny3 = 1;
  nz0 = 1, nz1 = 1, nz2 = 1, nz3 = 1;

  String mef = "/";
  mef += ACCEL_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;

  if ( ! SD.exists( mef ) )
  {
    logger.info( F( "Creating wrist gesture sensing directory " ) );
    logger.info( mef );
    storage.createDir( SD, mef.c_str() );
  }
  else
  {
    logger.info( F( "Accel directory exists" ) );    
  }

  state = 0;
  co = 0;

  dx = 0;
  dy = 0;
  dz = 0;

  threshold = 0;
}

int Accelerometer::getRecentGesture()
{
  if ( ( ( millis() - recenttimer ) > recentdelay ) )
  {
    recenttimer = millis();

    recentGesture = 0;
  }

  return recentGesture;
}

void Accelerometer::setTraining( bool mode )
{
  trainingMode = mode;

  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;
  storage.deleteFile( SD, mef.c_str() );
}

boolean Accelerometer::test()
{
  // waits up to 5 seconds to get data from the Accelerometer module

  long time = millis();
  while ( millis() < time + 5000 )
  {
    if ( myIMU.readFloatAccelX() > 0 ) 
    {
      return true;
    }
  }
  return false;    
}

// For Parallex effect
// returns X axis value from 0 to 5

float Accelerometer::getXreading()
{
  return myIMU.readFloatAccelX();
}

// Get values from accelermoter, apply scale and filter

bool Accelerometer::getAccelValues()
{
  nx = myIMU.readFloatAccelX() * scaler ;
  ny = myIMU.readFloatAccelY() * scaler ;
  nz = myIMU.readFloatAccelZ() * scaler ;

  nx += 300;
  ny += 300;
  nz += 300;

  return true;
}

// Returns true when accelerator is moving

bool Accelerometer::detectStartOfGesture()
{
  if ( ( ( millis() - movetimer ) > windowtime ) || firsttime )
  {
    movetimer = millis();
    firsttime = false; 
    getAccelValues();

    nx0 = nx;
    nx1 = nx;
    nx2 = nx;
    nx3 = nx;

    ny0 = ny;
    ny1 = ny;
    ny2 = ny;
    ny3 = ny;

    nz0 = nz;
    nz1 = nz;
    nz2 = nz;
    nz3 = nz;
    
    return false;
  }

  getAccelValues();
 
  nx0 = nx1;
  nx1 = nx2;
  nx2 = nx;
  nx3 =  ( nx0 + nx1 + nx2 ) / 3 ;

  ny0 = ny1;
  ny1 = ny2;
  ny2 = ny;
  ny3 =  ( ny0 + ny1 + ny2 ) / 3 ;

  nz0 = nz1;
  nz1 = nz2;
  nz2 = nz;
  nz3 =  ( nz0 + nz1 + nz2 ) / 3 ;

  if ( ( nx < ( nx3 * ( 1 - tollerance ) ) ) || ( nx > ( nx3 * ( 1 + tollerance ) ) ) ) 
  {
    firsttime = true;  
    movetimer = millis();
    return true;
  }

  if ( ( ny < ( ny3 * ( 1 - tollerance ) ) ) || ( ny > ( ny3 * ( 1 + tollerance ) ) ) )
  {
    firsttime = true;  
    movetimer = millis();
    return true;
  }

  if ( ( nz < ( nz3 * ( 1 - tollerance ) ) ) || ( nz > ( nz3 * ( 1 + tollerance ) ) ) )
  {
    firsttime = true;  
    movetimer = millis();
    return true;
  }

  return false;
}

/*

  Generated by OpenAI ChatGPT 3.5 on March 26, 2024 using this prompt:
  Hi, are you familiar with Dynamic Time Warping DTW?
  Please write a DTW demonstration program in C/C++ to compile on Arduino IDE for an ESP32S3 processor

  ChatGPT: "This code defines a function DTWdistance() that calculates the Dynamic Time Warping (DTW) 
  distance between two sequences of equal length. The min() function finds the minimum of three values, 
  and the distance() function calculates the Euclidean distance between two points in the sequences. The 
  setup() function initializes Serial communication and calculates the DTW distance for two example 
  sequences (seq1 and seq2). Finally, the DTW distance is printed to the Serial monitor."

  I have an accelerometer that gives me x, y, and z values. Please change the DTW code to support 3 axis values

  ChatGPT: "I've modified the distance() function to calculate the Euclidean distance between two points in 3D space. 
  The DTWdistance() function now takes 3D sequences as input and computes the DTW distance accordingly. Finally, I've 
  updated the example sequences (seq1 and seq2) to be 3D sequences for demonstration purposes."

 */

// Function to calculate the minimum of three values
float Accelerometer::DTWmin(float a, float b, float c) {
  if ( ( a < b ) && ( a < c ) ) return a;
  if ( ( b < c ) && ( b < a ) ) return b;
  if ( ( c < b ) && ( c < a ) ) return c;
  return a;
}

// Function to calculate the Euclidean distance between two points in a sequence
float Accelerometer::DTWdistance(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return sqrt( ( dx * dx ) + ( dy * dy ) + ( dz * dz ) );
}

float Accelerometer::DTWgpt( float seq1[][3], float seq2[][3], int len )
{
  // Initialize the cost matrix

  for ( int i = 0; i < len; i++ )
  {
    for ( int j = 0; j < len; j++ )
    {
      cost[i][j] = 0;
    }
  }

  // Initialize the first row and column of the cost matrix
  cost[0][0] = DTWdistance(seq1[0][0], seq1[0][1], seq1[0][2], seq2[0][0], seq2[0][1], seq2[0][2]);

  for (int i = 1; i < len; i++) 
  {
    cost[i][0] = cost[i-1][0] + DTWdistance(seq1[i][0], seq1[i][1], seq1[i][2], seq2[0][0], seq2[0][1], seq2[0][2]);
    cost[0][i] = cost[0][i-1] + DTWdistance(seq1[0][0], seq1[0][1], seq1[0][2], seq2[i][0], seq2[i][1], seq2[i][2]);
  }

  // Fill in the rest of the cost matrix
  for (int i = 1; i < len; i++) 
  {
    for (int j = 1; j < len; j++)
    {
      float dist = DTWdistance(seq1[i][0], seq1[i][1], seq1[i][2], seq2[j][0], seq2[j][1], seq2[j][2]);
      cost[i][j] = dist + DTWmin(cost[i-1][j], cost[i][j-1], cost[i-1][j-1]);
    }
  }

  // Return the DTW distance
  return cost[len-1][len-1];
}

/* Calculates a single percentage value from a comparison of accelerometer x, y, z template values
   100% means the templates are a match, anything lower is a measurement of their differences
*/

float Accelerometer::timeWarp( int gestureIndex, int typen )
{
  float xcount = 0;
  float ycount = 0;
  float zcount = 0;
  float xper = 0;
  float yper = 0;
  float zper = 0;
  float frames = maxframes;

  for ( int i = 0; i < maxframes; i++ )
  {
    xper = accxt[ i ] / accx[ gestureIndex ][ i ][ typen ];
    if ( ( xper > 0.90 ) && ( xper < 1.10 ) ) xcount++;

    yper = accyt[ i ] / accy[ gestureIndex ][ i ][ typen ];
    if ( ( yper > 0.90 ) && ( yper < 1.10 ) ) ycount++;

    zper = acczt[ i ] / accz[ gestureIndex ][ i ][ typen ];
    if ( ( zper > 0.90 ) && ( zper < 1.10 ) ) zcount++;    
  }

  // Then average the axis counts

  return ( ( ( xcount / frames ) + ( ycount / frames ) + ( zcount / frames ) ) / 3 );
}

/*
 * Save recorded gestures to Accelerometer Gesture (.ages) binary file format
 * /REFLECTIONS/agesture/fantastic4.ages
*/

bool Accelerometer::saveGestures()
{
  String mef = "/";
  mef += ACCEL_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;

  File myFile = SD.open( mef, FILE_WRITE );
  if ( myFile )
  {
    logger.info( F( "SD file opened for write: " ) );
    logger.info( mef );
  }
  else
  {
    logger.error( F( "Error opening file for writing: " ) );
    logger.error( mef );
    return false;
  }

  // Write VersionNumber to the file
  int version = ages_version;
  myFile.write((uint8_t*) &version, sizeof( version ));

  // gesture type descriptions
  for (int i = 0; i < gesturetypes; i++)
  {
    String gesturetype;
    if ( i == 0 ) { gesturetype = type1; }
    if ( i == 1 ) { gesturetype = type2; }
    if ( i == 2 ) { gesturetype = type3; }
    if ( i == 3 ) { gesturetype = type4; }

    myFile.write( (uint8_t *) gesturetype.c_str(), (int) gesturetype.length() + 1); // Include null terminator
  }

  // Write accx, accy, and accz arrays to the file
  myFile.write((uint8_t*)accx, sizeof(accx));
  myFile.write((uint8_t*)accy, sizeof(accy));
  myFile.write((uint8_t*)accz, sizeof(accz));

  // Close the file
  myFile.close();

  logger.info("Data saved to file.");

  return true;
}

/*
 * Load recorded gestures from Accelerometer Gesture (.ages) binary file format
 * /REFLECTIONS/agesture/fantastic4.ages
*/

bool Accelerometer::loadGestures()
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;

  int VersionNumber;

  // Open file for reading
  File file = SD.open( mef, FILE_READ);
  if (!file) {
    logger.error("Accelerometer error opening file for reading.");
    return false;
  }

  // Read VersionNumber from the file
  file.read((uint8_t*)&VersionNumber, sizeof(VersionNumber));

  //logger.info( "Ages loading, version number " );
  //logger.info( String( VersionNumber ) );

  // Read gesture types from the file
  //logger.info( "Types: ");
  for ( int i = 0; i < gesturetypes; i++ )
  {
    char buffer[21]; // Maximum string length + 1 for null terminator
    file.readBytesUntil('\0', buffer, sizeof(buffer));
    //logger.info( String( buffer ) );
  }

  // Read accx, accy, and accz arrays from the file
  file.read((uint8_t*)accx, sizeof(accx));
  file.read((uint8_t*)accy, sizeof(accy));
  file.read((uint8_t*)accz, sizeof(accz));

  logger.info( "Accelerometer loading done" );

  file.close();

  gesturesloaded = true;

  return true;
}

void Accelerometer::loop()
{
  // Training mode, records gesture templates, stores to .ages file (Accelerometer Gesture Template)

  // Record 10 templates, then stop

  if ( ( trainingMode ) && ( ! recordingTemplate ) ) 
  {
    if ( ! firstnotice )
    {
      firstnotice = true;

      logger.info("Template recording, ");
      logger.info( String( gesturecount ) );
      logger.info( ", ");

      if ( typecounter == 0 ) { logger.info( type1 ); }
      if ( typecounter == 1 ) { logger.info( type2 ); }
      if ( typecounter == 2 ) { logger.info( type3 ); }
      if ( typecounter == 3 ) { logger.info( type4 ); }
      logger.info( " gesture" );
    }

    if ( detectStartOfGesture() )
    {
      recordtimer = millis();
      recordingTemplate = true;
      recordi = 0;
    }
  }
  
  if ( trainingMode && recordingTemplate )
  {
    if ( ( ( millis() - recordtimer ) > framedelay ) )
    {
      recordtimer = millis();

      getAccelValues();

      accx[gesturecount][recordi][typecounter] = nx;
      accy[gesturecount][recordi][typecounter] = ny;
      accz[gesturecount][recordi][typecounter] = nz;
      
      recordi++;
      if ( recordi >= maxframes )
      {
        recordingTemplate = false;
        recordi = 0;
        haptic.playEffect(70);  // Transition Ramp Down Long Smooth 1 – 100 to 0%
        //haptic.playEffect(14);  // 14 Strong Buzz
        logger.info( "recording done" );
        firstnotice = false;

        gesturecount++;
        if ( gesturecount >= maxgestures )
        {
          gesturecount = 0;

          typecounter++;
          if ( typecounter >= gesturetypes )
          {
            trainingMode = false;
            gesturesloaded = true;
            gesturestart = false;
            typecounter = 0;

            saveGestures();

            // For testing, dump contents of new gesture file
            /*
            String mef = "/";
            mef += NAND_BASE_DIR;
            mef += "/";
            mef += ACCEL_BASE_DIR;
            mef += "/";
            mef += ACCEL_BASE_FILE;
            mef += ACCEL_BASE_EXT;
            File gfile = SD.open( mef );
            if ( ! gfile )
            {
              logger.info( "Unable to open new gesture file for hexdump " );
              logger.info( mef );
            }
            else
            {
              logger.info( "Open new gesture file for hexdump " );
              logger.info( mef );
              utils.hexDump( gfile );
            }
            */

          }
        }
      }
    }
  }

  // Detect recognized gestures, show results

  if ( ( ! trainingMode ) && ( gesturesloaded ) )
  {
    if ( ! gesturestart )
    {
      if ( ! firstnotice )
      {
        //logger.info("Make a gesture... ");
        firstnotice = true;
      }

      if ( detectStartOfGesture() )
      {
        //logger.info("Sensing gesture");
        gesturestart = true;
        recordtimer = millis();
        recordi = 0;
      }
    }
    else
    {
      // Record current accel into template

      if ( ( ( millis() - recordtimer ) > framedelay ) )
      {
        recordtimer = millis();

        getAccelValues();

        accxt[recordi] = nx;
        accyt[recordi] = ny;
        acczt[recordi] = nz;
        
        recordi++;
        if ( recordi >= maxframes )
        {
          gesturestart = false;
          firstnotice = false;
          
          //logger.info("Comparing gestures");

          // Pick a winner, the DTWgpt way

          //int dtwtype = 0;
          //float dtwbottomval = 0;
          float dtwhold = 0;

          for ( int t = 0; t < gesturetypes; t++ )
          {
            dtwlow[ t ] = 1000000;
            dtwavg[ t ] = 0;

            /*
            logger.info( "DTWgpt accelerometer gesture " );
            if ( t == 0 ) { logger.info( type1 ); }
            if ( t == 1 ) { logger.info( type2 ); }
            if ( t == 2 ) { logger.info( type3 ); }
            if ( t == 3 ) { logger.info( type4 ); }
            logger.info( "\t" );
            */

            for ( int c = 0; c < maxgestures; c++ )
            {

              for ( int m = 0; m < maxframes; m++ )
              {
                seq1[ m ][ 0 ] = accxt[ m ];
                seq1[ m ][ 1 ] = accyt[ m ];
                seq1[ m ][ 2 ] = acczt[ m ];

                seq2[ m ][ 0 ] = accx[ c ][ m ][ t ];
                seq2[ m ][ 1 ] = accy[ c ][ m ][ t ];
                seq2[ m ][ 2 ] = accz[ c ][ m ][ t ];
              }

              dtwhold = DTWgpt(seq1, seq2, maxframes);
              
              dtwavg[ t ] += dtwhold;
              
              if ( dtwhold < dtwlow[ t ] )
              {
                dtwlow[ t ] = dtwhold;
              }

              //logger.info( String( dtwhold ) );
              //logger.info( ",\t" );
            }

            //float myvg = dtwavg[ t ] / maxgestures;
            //logger.info( String( myvg ) );
            //logger.info( ",\t" );
            //logger.info( String( dtwlow[ t ] ) );
            //logger.info( ",\t" );
          }

          int dtwlowtype = 0;
          int dtwbotval = 1000000;
          
          for ( int v = 0; v < gesturetypes; v++ )
          {
            if ( ( dtwavg[ v ] / maxgestures ) < dtwbotval )
            {
              dtwlowtype = v;
              dtwbotval = dtwavg[ v ] / maxgestures;
            }
          }

          String mef = F( "DTWgpt, gesture recognized: " );
          if ( dtwbotval < 5000 )
          {
            if ( dtwlowtype == 0 )
            {
              recentGesture = 1;
              mef += type1; 
            }

            if ( dtwlowtype == 1 )
            {
              recentGesture = 2;
              mef += type2; 
            }

            if ( dtwlowtype == 2 )
            {
              recentGesture = 3;
              mef += type3; 
            }

            if ( dtwlowtype == 3 )
            {
              recentGesture = 4;
              mef += type4; 
            }

            logger.info( mef );
          }
          else
          {
            recentGesture = 0;
            logger.info( "Inconclusive" );
          }

          // Pick a winner, the Sum Of Averages and Highest Average way

          /*

          float gesperc = 0;

          int toptype = 0;
          float topval = 0;

          int topsy = 0;
          float topsyval = 0;

          for ( int t = 0; t < gesturetypes; t++ )
          {
            aavgs[ t ] = 0;
            tophigh[ t ] = 0;

            logger.info( "Accelerometer gesture " );
            if ( t == 0 ) { logger.info( type1 ); }
            if ( t == 1 ) { logger.info( type2 ); }
            if ( t == 2 ) { logger.info( type3 ); }
            if ( t == 3 ) { logger.info( type4 ); }
            logger.info( "\t" );

            for ( int c = 0; c < maxgestures; c++ )
            {
              gesperc = timeWarp( c, t );
              logger.info( gesperc );
              logger.info( ",\t" );

              aavgs[ t ] += gesperc;

              if ( gesperc > tophigh[ t ] )
              {
                tophigh[ t ] = gesperc;
              }
            }
            //logger.info( " " );

            if ( tophigh[ t ] > topsyval  )
            {
              topsy = t;
              topsyval = tophigh[ t ];
            }

          }

          for ( int v = 0; v < gesturetypes; v++ )
          {
            if ( aavgs[ v ] > topval )
            {
              toptype = v;
              topval = aavgs[ v ];
            }
          }
          topval = topval / maxgestures;

          if ( topval > 0.55 )
          {
            logger.info( "Sum of averages, gesture recognized: " );
            if ( toptype == 0 ) { logger.info( type1 ); }
            if ( toptype == 1 ) { logger.info( type2 ); }
            if ( toptype == 2 ) { logger.info( type3 ); }
            if ( toptype == 3 ) { logger.info( type4 ); }
            logger.info( " " );
          }
          else
          {
            logger.info( "Sum of averages, inconclusive " );
          }

          if ( topsyval > 0.55 )
          {
            logger.info( "Highest average, gesture recognized: " );
            if ( topsy == 0 ) { logger.info( type1 ); }
            if ( topsy == 1 ) { logger.info( type2 ); }
            if ( topsy == 2 ) { logger.info( type3 ); }
            if ( topsy == 3 ) { logger.info( type4 ); }
            logger.info( " " );
          }
          else
          {
            logger.info( "Highest average, inconclusive " );
          }

          */

        }
      }
    }
  }

}
