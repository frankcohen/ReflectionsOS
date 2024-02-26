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

#include "Accellerometer.h"

extern Storage storage;   // Defined in ReflectionsOfFrank.ino
extern Haptic haptic;
extern Utils utils;

Accellerometer::Accellerometer(){}

void Accellerometer::begin()
{ 
  LIS.begin( Wire ); //IIC init dafault :0x18
  delay(100);
  
  LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
  LIS.setHighSolution(true); //High solution enable

  movetimer = millis();
  detecttimer = millis();
  firsttime = true;
  movecount = 1;
  olddtw = 0;
  recordi = 0;
  
  gesturenumber = 0;
  gesturecount = 0;
  cmpcount = 0; 
  gesturesloaded = false;
  currentreading = false;
  gesturestart = false;
  recordingTemplate = false;

  nx0 = 1, nx1 = 1, nx2 = 1, nx3 = 1, nx4 = 1;
  ny0 = 1, ny1 = 1, ny2 = 1, ny3 = 1, ny4 = 1;
  nz0 = 1, nz1 = 1, nz2 = 1, nz3 = 1, nz4 = 1;

  // Make sure /REFECTIONS/agesture/ + gesturename directory exists

  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;

  if ( ! SD.exists( mef ) )
  {
    Serial.print( F( "Creating accellerometer directory " ) );
    Serial.println( mef );
    storage.createDir( SD, mef.c_str() );
  }
  else
  {
    Serial.print( mef );
    Serial.println( F( " exists" ) );    
  }

  firsttime = true;
  movecount = 1;

  nx0 = 1, nx1 = 1, nx2 = 1, nx3 = 1, nx4 = 1;
  ny0 = 1, ny1 = 1, ny2 = 1, ny3 = 1, ny4 = 1;
  nz0 = 1, nz1 = 1, nz2 = 1, nz3 = 1, nz4 = 1;

  timer = 0;

  state = 0;
  co = 0;

  DTW[ maxframes ][ maxframes ];

  dx = 0;
  dy = 0;
  dz = 0;

  threshold = 0;
}

void Accellerometer::setTraining( bool mode )
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

boolean Accellerometer::test()
{
  // waits up to 5 seconds to get data from the accellerometer module

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

// Start DTW threshold processing

float Accellerometer::DTW_THRESHOLD( int gesturenum )
{
  float scale = 0.5;
  float dir = 0;

  for ( int8_t i = 0; i < maxframes ; i++ )
  {
    if (i == 0)
    {
      dx = accx[gesturenum][i] - accxt[i];
      dy = accy[gesturenum][i] - accyt[i];
      dz = accz[gesturenum][i] - acczt[i];
 
      dir = ( accx[gesturenum][i] * accxt[i] + accy[gesturenum][i] * accyt[i] + accz[gesturenum][i] * acczt[i] )
        / ( DTWNORM( accx[gesturenum][i], accy[gesturenum][i], accz[gesturenum][i]) * DTWNORM( accxt[i], accyt[i], acczt[0] ) + 0.0000001 );
 
      DTW[i][i] = ( 1-scale*dir ) * DTWNORM( dx,dy,dz );
    }
    else
    {
      dx = accx[gesturenum][i] - accxt[0];
      dy = accy[gesturenum][i] - accyt[0];
      dz = accz[gesturenum][i] - acczt[0];
 
      dir = ( accx[gesturenum][i] * accxt[0] + accy[gesturenum][i] * accyt[0] + accz[gesturenum][i] * acczt[0])
        / ( DTWNORM( accx[gesturenum][i], accy[gesturenum][i], accz[gesturenum][i]) * DTWNORM( accxt[0], accyt[0], acczt[0] ) + 0.0000001);
 
      DTW[i][0] = ( 1-scale*dir ) * DTWNORM( dx, dy, dz ) + DTW[i-1][0];

      dx = accx[gesturenum][0] - accxt[i];
      dy = accy[gesturenum][0] - accyt[i];
      dz = accz[gesturenum][0] - acczt[i];
 
      dir = ( accx[gesturenum][0] * accxt[i] + accy[gesturenum][0] * accyt[i] + accz[gesturenum][0] * acczt[i] )
        / ( DTWNORM( accx[gesturenum][0], accy[gesturenum][0], accz[gesturenum][0]) * DTWNORM( accxt[i], accyt[i], accz[gesturenum][i] ) + 0.0000001);
 
      DTW[0][i] = ( 1-scale * dir ) * DTWNORM( dx, dy, dz ) + DTW[0][i-1];
    }
  }

  for ( int8_t i = 1 ; i < maxframes ; i++)
  {
    for ( int8_t j = 1 ; j < maxframes ; j++)
    {
      dx = accx[gesturenum][i] - accxt[j];
      dy = accy[gesturenum][i] - accyt[j];
      dz = accz[gesturenum][i] - acczt[j];
        
      dir = ( accx[gesturenum][i] * accxt[j] + accy[gesturenum][i] * accyt[j] + accz[gesturenum][i]*acczt[j])
        / ( DTWNORM(accx[gesturenum][i], accy[gesturenum][i], accz[gesturenum][i]) * DTWNORM( accxt[j], accyt[j], acczt[j] ) + 0.0000001);  

      DTW[i][j] = ( 1 - scale * dir ) * DTWNORM( dx,dy,dz ) + DTWMIN( DTW[ i-1 ][ j ],DTW[ i ][ j-1 ], DTW[ i-1 ][ j-1 ]);    
        
    }
  }

  // Back track starts

  int8_t i = 49;
  int8_t j = 49;
  int8_t count = 0;
  float d[ 100 ];

  while( true )
  {
    if ( i>0 && j>0 )
    {
      float m = DTWMIN( DTW[ i-1 ][ j ], DTW[ i ][ j-1 ], DTW[ i-1 ][ j-1 ] );

      if ( m == DTW[i-1][j-1] )
      {
        d[count] = DTW[i][j] - DTW[i-1][j-1];
        i = i-1;
        j = j-1;
        count++;
      }
      else if ( m == DTW[i][j-1] )
      {
        d[count] = DTW[i][j] - DTW[i][j-1];
        j = j-1;
        count++;
      }
      else if ( m == DTW[i-1][ j ] )
      {
        d[count] = DTW[i][j] - DTW[i-1][j];
        i = i-1;
        count++;
      }
    }

    else if ( i == 0 && j == 0 )
    {
      d[count] = DTW[i][j];
      count++;
      break;
    }

    else if ( i == 0 )
    {
      d[count] = DTW[i][j] - DTW[i][j-1];
      j = j-1;
      count++;
    }

    else if ( j == 0 )
    {
      d[count] = DTW[i][j] - DTW[i-1][j];
      i = i-1;
      count++;
    }
  }
  
  // Calculate cost

  float mean = 0;
  for (int i = 0 ; i < count ; i++)
  {    
    mean += d[i];
  }
  mean = mean / count;
    
//    float variance = 0;
//    for (int i = 0 ; i < count ; i++){
//      variance += ( d[i] - mean )*( d[i] - mean ) / count;
//    }

  /*
  Serial.print("Threshold: ");
  Serial.print( threshold );
  Serial.print(", Cost: ");
  Serial.println( mean );
  */

  return mean;
}

// Get values from accelermoter, apply scale and filter

bool Accellerometer::getAccelValues()
{
  nx = LIS.getAccelerationX() * scaler ;
  ny = LIS.getAccelerationY() * scaler ;
  nz = LIS.getAccelerationZ() * scaler ;

  if ( 
    ( ( nx > scalemax ) || ( nx < scalemin ) ) ||
    ( ( ny > scalemax ) || ( ny < scalemin ) ) ||
    ( ( nz > scalemax ) || ( nz < scalemin ) )
  )
  {
    Serial.print( "ignoring " );
    Serial.print( "\tX = ");
    Serial.println( nx );
    Serial.print( "\tY = ");
    Serial.println( ny );
    Serial.print( "\tZ = ");
    Serial.println( nz );

    return false;
  }

  return true;
}

float Accellerometer::DTWNORM( float dx, float dy, float dz){
  return sqrt(dx*dx + dy*dy + dz*dz);
}

float Accellerometer::DTWMIN( float &a, float &b ,float &c )
{
  if ( a < b ){
    if ( a < c ) return a;
    else return c;
  }
  else {
    if ( b < c ) return b;
    else return c;
  }
}

// Returns true when accelerator is moving

bool Accellerometer::detectStartOfGesture()
{
  if ( ( ( millis() - movetimer ) > windowtime ) || ( firsttime ) )
  {
    movetimer = millis();
    firsttime = false;

    if ( ! getAccelValues() )
    {
      firsttime = true;
      return false;
    }

    nx0 = nx; 
    ny0 = ny;
    nz0 = nz;
    
    return false;
  }

  if ( ! getAccelValues() ) return false;
  
  nx1 = nx2;
  nx2 = nx3;
  nx3 = nx;
  nx4 =  ( nx1 + nx2 + nx3 ) / 3 ;

  ny1 = ny2;
  ny2 = ny3;
  ny3 = ny;
  ny4 =  ( ny1 + ny2 + ny3 ) / 3 ;

  nz1 = nz2;
  nz2 = nz3;
  nz3 = nz;
  nz4 =  ( nz1 + nz2 + nz3 ) / 3 ;

  movecount++;

  bool popx = false, popy = false, popz = false;

  float meow = nx0 + ( nx0 * ( tollerance / 2 ) );
  float wolf = nx0 - ( nx0 * ( tollerance / 2 ) );

  if ( nx4 > 0 )
  {
    if ( ( nx4 < meow ) && ( nx4 > wolf ) ) popx = true;
  }
  else
  {
    if ( ( nx4 > meow ) && ( nx4 < wolf ) ) popx = true;
  }

  meow = ny0 + ( ny0 * ( tollerance / 2 ) );
  wolf = ny0 - ( ny0 * ( tollerance / 2 ) );

  if ( ny4 > 0 )
  {
    if ( ( ny4 < meow ) && ( ny4 > wolf ) ) popy = true;
  }
  else
  {
    if ( ( ny4 > meow ) && ( ny4 < wolf ) ) popy = true;
  }

  meow = nz0 + ( nz0 * ( tollerance / 2 ) );
  wolf = nz0 - ( nz0 * ( tollerance / 2 ) );

  if ( nz4 > 0 )
  {
    if ( ( nz4 < meow ) && ( nz4 > wolf ) ) popz = true;
  }
  else
  {
    if ( ( nz4 > meow ) && ( nz4 < wolf ) ) popz = true;
  }

  if ( popx || popy || popz )
  {
    //firsttime = true;
    return true;
  }

  return false;
}

/*
  Save gesture to Accellerometer Gesture (.ages) binary file format
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

bool Accellerometer::saveGesture()
{
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_DIR;
  mef += "/";
  mef += ACCEL_BASE_FILE;
  mef += ACCEL_BASE_EXT;

  File myFile = SD.open( mef, FILE_APPEND );
  if ( myFile )
  {
    Serial.print( F( "SD file opened for write: " ) );
    Serial.println( mef );
  }
  else
  {
    Serial.print( F( "Error opening new file for writing: " ) );
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

  int gesturelen = maxframes * 4 * 3;     // gestures are 3 axis of float values (4 bytes per float)
  
  Serial.print( "gesturelen = " );
  Serial.println( gesturelen );

  // gesture template data
  myFile.write( (uint8_t *) &accxt, gesturelen );

  myFile.close();

  return true;
}

bool Accellerometer::loadGesture()
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

/*
  Rating (.rating) binary file format

  gesture number - 4 bytes
  rating value (signed int) - 2 bytes
*/

bool Accellerometer::saveRatings()
{
  // Incomplete

  return true;
}

void Accellerometer::loop()
{
  // Training mode, records gesture templates, stores to .agt file (Accellerometer Gesture Template)

  // Record 10 templates, then stop

  if ( trainingMode && recordingTemplate )
  {
    if ( ( ( millis() - recordtimer ) > framedelay ) )
    {
      recordtimer = millis();

      // While nand doesn't work, record to memory

      //accxt[recordi] = LIS.getAccelerationX() * scaler;
      //accyt[recordi] = LIS.getAccelerationY() * scaler; 
      //acczt[recordi] = LIS.getAccelerationZ() * scaler;

      accx[gesturecount][recordi] = LIS.getAccelerationX() * scaler;
      accy[gesturecount][recordi] = LIS.getAccelerationY() * scaler; 
      accz[gesturecount][recordi] = LIS.getAccelerationZ() * scaler;

      if ( recordi++ >= maxframes )
      {
        recordingTemplate = false;
        recordi = 0;
        haptic.playEffect(70);  // Transition Ramp Down Long Smooth 1 – 100 to 0%

    /*        
        Serial.println( F( "Saving gesture" ) );

        if ( !saveGesture() )
        {
          Serial.print( F( "saveGesture: error writing gesture" ) );
          recordingTemplate = false;
        }
    */

        if ( gesturecount++ > maxgestures )
        {
          trainingMode = false;

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

  if ( ( ( trainingMode ) && ( ! recordingTemplate ) ) && ( ( millis() - detecttimer ) > detectdelay ) )
  {
    detecttimer = millis();

    if ( detectStartOfGesture() )
    {
      recordingTemplate = true;
      recordtimer = millis();
      recordi = 0;
      Serial.print("Template recording ");
      Serial.println( gesturecount );
      haptic.playEffect(14);  // 14 Strong Buzz
    }
  }

// Load gestures from sd

  gesturesloaded = true;
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

  // Detect recognized gestures, update rewards, show rewards

  if ( ( ! trainingMode ) && ( gesturesloaded ) && ( ! currentreading ) )
  {
    if ( ! gesturestart )
    {
      if ( detectStartOfGesture() )
      {
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

        // loop through recording all 50 frames here

        acczt[recordi] = LIS.getAccelerationX() * scaler;
        accyt[recordi] = LIS.getAccelerationY() * scaler; 
        acczt[recordi] = LIS.getAccelerationZ() * scaler;

        if ( recordi++ >= maxframes )
        {
          currentreading = true;
          olddtw = DTW_THRESHOLD( 0 );
          cmpcount = 0;
          haptic.playEffect(70);  // Transition Ramp Down Long Smooth 1 – 100 to 0%
        }
      }
    }
  }

  if ( currentreading )
  {
    if ( cmpcount < gesturecount )
    {
      newdtw = DTW_THRESHOLD( cmpcount );

      /*
      Serial.print( newdtw );
      Serial.print( "\t" );
      Serial.print( olddtw );
      Serial.print( "\t" );
      Serial.println( 100 * ( olddtw / newdtw ) );
      */

      float ratperc =100 * ( olddtw / newdtw );
      if ( ( ratperc > 0 ) &&
            ( ( ratperc < 115 ) && ( ratperc > 85 ) )
          )
      {
        Serial.print(" Recognized");
        Serial.print( "\t" );
        Serial.println( ratperc );

        // bump rating for this gesture template

        reward[ cmpcount ] ++;

        // Show rewards board in serial port

        for ( int m = 0; m<maxgestures; m++ )
        {
          Serial.println( "" );
          Serial.println( "Rewards board" );
          Serial.print( "G" );
          Serial.print( cmpcount );
          Serial.print( "=" );
          Serial.println( reward[m] );
        }
      }

      haptic.playEffect(58);  // Transition Click 1 – 100%      

      cmpcount++;
    }
    else
    {
      currentreading = false;
      gesturestart = false;
    }
  }
}
