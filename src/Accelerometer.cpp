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
  https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR

  Modified from Federico Terzi experiment at
  https://github.com/CieloStrive/GESTURE-RECOGNITION-DYNAMIC-TIME-WARPING/tree/master

  Future:
  Add a gesture pruning service to remove lowest rated gestures

*/

#include "Accelerometer.h"

extern Storage storage;   // Defined in ReflectionsOfFrank.ino
extern Haptic haptic;
extern Utils utils;

Accelerometer::Accelerometer(){}

void Accelerometer::begin()
{ 
  LIS.begin( Wire ); //IIC init dafault :0x18
  delay(100);
  
  LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
  LIS.setHighSolution(true); //High solution enable

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

  // Make sure /REFECTIONS/agesture/ + gesturename directory exists

  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;

  if ( ! SD.exists( mef ) )
  {
    Serial.print( F( "Creating Accelerometer directory " ) );
    Serial.println( mef );
    storage.createDir( SD, mef.c_str() );
  }
  else
  {
    Serial.print( mef );
    Serial.println( F( " exists" ) );    
  }

  state = 0;
  co = 0;

  dx = 0;
  dy = 0;
  dz = 0;

  threshold = 0;
}

void Accelerometer::setTraining( bool mode )
{
  trainingMode = mode;

  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;
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
    if ( LIS )
    {
      return true;
    }
  }
  return false;    
}

// Get values from accelermoter, apply scale and filter

bool Accelerometer::getAccelValues()
{
  nx = LIS.getAccelerationX() * scaler ;
  ny = LIS.getAccelerationY() * scaler ;
  nz = LIS.getAccelerationZ() * scaler ;

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
  float weightedCount = 0;

  for ( int i = 0; i < maxframes; i++ )
  {
    xper = accxt[ i ] / accx[ gestureIndex ][ i ][ typen ];
    if ( ( xper > 0.90 ) && ( xper < 1.10 ) ) xcount++;

    yper = accyt[ i ] / accy[ gestureIndex ][ i ][ typen ];
    if ( ( yper > 0.90 ) && ( yper < 1.10 ) ) ycount++;

    zper = acczt[ i ] / accz[ gestureIndex ][ i ][ typen ];
    if ( ( zper > 0.90 ) && ( zper < 1.10 ) ) zcount++;

    /*
    Serial.print( "timeWarp " );
    Serial.print( gestureIndex );
    Serial.print( ", " );
    Serial.print( xper );
    Serial.print( ", " );
    Serial.print( yper );
    Serial.print( ", " );
    Serial.print( zper );
    Serial.print( ", " );
    Serial.print( xcount );
    Serial.print( ", " );
    Serial.print( ycount );
    Serial.print( ", " );
    Serial.print( zcount );
    Serial.print( ", " );
    Serial.print( accxt[ i ] );
    Serial.print( ", " );
    Serial.print( accx[ gestureIndex ][ i ] );
    Serial.print( ", " );
    Serial.print( accyt[ i ] );
    Serial.print( ", " );
    Serial.print( accy[ gestureIndex ][ i ] );
    Serial.print( ", " );
    Serial.print( acczt[ i ] );
    Serial.print( ", " );
    Serial.print( accz[ gestureIndex ][ i ] );
    Serial.println( " " );
    */
    
  }

  // Then average the axis counts

  return ( ( ( xcount / frames ) + ( ycount / frames ) + ( zcount / frames ) ) / 3 );
}

/*
  Save gesture to Accelerometer Gesture (.ages) binary file format
  https://siliz4.github.io/guides/tutorials/2020/05/21/guide-on-binary-files-cpp.html

  gesture category - 2 bytes
    0 - yes
    1 - cancel, no, wrong
    2 - next
    3 - previous
  gesture number - 4 bytes
  gesture name length - 2 bytes
  gesture name - variable
  gesture length - 4 bytes
  gesture template data - variable
*/

/*
 * Store recorded gestures to /REFLECTIONS/agesture/fantastic4.ages
*/

bool Accelerometer::saveGesture()
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;

  File myFile = SD.open( mef, FILE_WRITE );
  if ( myFile )
  {
    Serial.print( F( "SD file opened for write: " ) );
    Serial.println( mef );
  }
  else
  {
    Serial.print( F( "Error opening file for writing: " ) );
    Serial.println( mef );
    return false;
  }

  // version number

  // gesture type descriptions

  // gesture array

  


  int gesturecategory = 1;
  myFile.write( (uint8_t *) &gesturecategory, 2 );

  // gesture number - 2 bytes
  myFile.write( (uint8_t *) &gesturenumber, 2 );
  gesturenumber++;

  String gesturename = "cancel";
  int gesturenamelen = gesturename.length();
  // gesture length - 2 bytes
  myFile.write( (uint8_t *) &gesturenamelen, gesturenamelen );

  // gesture name - variable
  myFile.write( (uint8_t *) &gesturename, (int) gesturenamelen );

  // gestures are 3 axis of float values (4 bytes per float):
  // float accx[maxgestures][maxframes][gesturetypes];
  // float accy[maxgestures][maxframes][gesturetypes];
  // float accz[maxgestures][maxframes][gesturetypes];

  int gesturelen = maxframes * maxgestures * gesturetypes * 4;
  
  Serial.print( "gesturelen = " );
  Serial.println( gesturelen );

  // gesture template data
  myFile.write( (uint8_t *) &accxt, gesturelen );

  myFile.close();

  return true;
}

bool Accelerometer::loadGesture()
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;

  File myFile = SD.open( mef, FILE_READ );
  if ( myFile )
  {
    Serial.print( F( "SD file opened for read: " ) );
    Serial.println( mef );
  }
  else
  {
    Serial.print( F( "Error opening new file for read: " ) );
    Serial.println( mef );
    return false;
  }

  /*
  gesture category - 2 bytes
    0 - yes
    1 - cancel, no, wrong
    2 - next
    3 - previous
  */

  int gesturecategory = 1;
  if ( myFile.read( (uint8_t *) &gesturecategory, sizeof( int ) ) == 0 )
  {
    return false;
  }

  // gesture number - 4 bytes
  int gn = 0;
  if ( myFile.read( (uint8_t *) &gn, sizeof( int ) ) == 0 )
  {
    return false;
  }

  String gesturename = "cancel";
  int gesturenamelen = gesturename.length();
  if ( myFile.read( (uint8_t *) &gesturenamelen, sizeof( int ) ) == 0 )
  {
    return false;
  }

  // gesture length - 4 bytes
  if ( myFile.read( (uint8_t *) &gesturename, gesturenamelen ) == 0 )
  {
    return false;
  }

  int gesturelen = maxframes * 4 * 3 * maxgestures;     // gestures are 3 axis of float values (4 bytes per float)
  
  // gesture template data
  if ( myFile.read( (uint8_t *) &accxt, gesturelen ) == 0 )
  {
    return false;
  }

  myFile.close();

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

      Serial.print("Template recording, ");
      Serial.print( gesturecount );
      Serial.print( ", ");

      if ( typecounter == 0 ) { Serial.print( type1 ); }
      if ( typecounter == 1 ) { Serial.print( type2 ); }
      if ( typecounter == 2 ) { Serial.print( type3 ); }
      if ( typecounter == 3 ) { Serial.print( type4 ); }
      Serial.println( " gesture" );
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
        Serial.println( "recording done" );
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
          }

          /*
          Serial.println( "accx,y,z values" );
          for ( int x = 0; x < gesturecount; x++ )
          {
            for ( int y = 0; y < maxframes; y++ )
            {
              for ( int z = 0; z < gesturetypes; z++ )
              {
                Serial.print( accx[x][y][z] );
                Serial.print( " " );
                Serial.print( accy[x][y][z] );
                Serial.print( " " );
                Serial.print( accz[x][y][z] );
                Serial.print( ",  " );
              }
            }
            Serial.println( "bop" );
          }
          */


          /*
          // For testing, dump contents of new gesture file
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
            Serial.print( "Unable to open new gesture file for hexdump " );
            Serial.println( mef );
          }
          else
          {
            Serial.print( "Open new gesture file for hexdump " );
            Serial.println( mef );
            utils.hexDump( gfile );
          }
        }
        */


        }
      }
    }
  }


// Load gestures from sd

/*
  if ( ( ! trainingMode ) && ( ! gesturesloaded ) )
  {
    gesturecount = 0;
    while ( ( gesturecount < maxgestures ) && loadGesture() )
    {
      gesturecount++;      
    }

    Serial.print( "Loaded " );
    Serial.print( gesturecount );
    Serial.println( " gestures" );

    gesturesloaded = true;
  }
*/

  // Detect recognized gestures, show results

  if ( ( ! trainingMode ) && ( gesturesloaded ) )
  {
    if ( ! gesturestart )
    {
      if ( ! firstnotice )
      {
        Serial.println("Make a gesture... ");
        firstnotice = true;
      }

      if ( detectStartOfGesture() )
      {
        Serial.println("Sensing gesture");
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


          // olddtw = DTW_THRESHOLD( 0 );
          //cmpcount = 0;
          
          Serial.println("Comparing gestures");

          /*    
          Serial.print( "acct values " );
          for ( int y = 0; y < maxframes; y++ )
          {
            Serial.print( accxt[y] );
            Serial.print( " " );
            Serial.print( accyt[y] );
            Serial.print( " " );
            Serial.print( acczt[y] );
            Serial.print( ",  " );
          }
          */

          float gesperc = 0;

          int toptype = 0;
          float topval = 0;

          int topsy = 0;
          float topsyval = 0;

          for ( int t = 0; t < gesturetypes; t++ )
          {
            aavgs[ t ] = 0;
            tophigh[ t ] = 0;

            Serial.print( "Accelerometer gesture " );
            if ( t == 0 ) { Serial.print( type1 ); }
            if ( t == 1 ) { Serial.print( type2 ); }
            if ( t == 2 ) { Serial.print( type3 ); }
            if ( t == 3 ) { Serial.print( type4 ); }
            Serial.print( "\t" );

            for ( int c = 0; c < maxgestures; c++ )
            {
              gesperc = timeWarp( c, t );
              Serial.print( gesperc );
              Serial.print( ",\t" );

              aavgs[ t ] += gesperc;

              if ( gesperc > tophigh[ t ] )
              {
                tophigh[ t ] = gesperc;
              }
            }
            Serial.println( " " );

            //Serial.println( aavgs[ t ] / maxgestures );
            //Serial.println( tophigh[ t ] );

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
            Serial.print( "Sum of averages, gesture recognized: " );
            if ( toptype == 0 ) { Serial.print( type1 ); }
            if ( toptype == 1 ) { Serial.print( type2 ); }
            if ( toptype == 2 ) { Serial.print( type3 ); }
            if ( toptype == 3 ) { Serial.print( type4 ); }
            Serial.println( " " );
          }
          else
          {
            Serial.println( "Sum of averages, inconclusive " );
          }

          if ( topsyval > 0.55 )
          {
            Serial.print( "Highest average, gesture recognized: " );
            if ( topsy == 0 ) { Serial.print( type1 ); }
            if ( topsy == 1 ) { Serial.print( type2 ); }
            if ( topsy == 2 ) { Serial.print( type3 ); }
            if ( topsy == 3 ) { Serial.print( type4 ); }
            Serial.println( " " );
          }
          else
          {
            Serial.println( "Highest average, inconclusive " );
          }

        }
      }
    }
  }

}
