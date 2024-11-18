/*
  Reflections is a hardware and software platform for building entertaining mobile experiences.

  Repository is at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS/)
  Includes board wiring directions, server side components, examples, support

  Licensed under GPL v3 Open Source Software
  (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
  Read the license in the license.txt file that comes with this code.

  Reflections board usees an (LIS3DHTR 3-Axis Accelerometer) to
  identify user gestures with their wrists and to wake the
  processor from sleep.

  Recognizes gestures by moving the Reflections board 

  Repository is at https://github.com/frankcohen/ReflectionsOS
  Includes board wiring directions, server side components, examples, support

  Supports multiple gestures and stores them on the SD/NAND
  Each gesture is 1.5 seconds, length is adjustable in the code

  Depends on:
  https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library

  Modified from Federico Terzi experiment at
  https://github.com/CieloStrive/GESTURE-RECOGNITION-DYNAMIC-TIME-WARPING/tree/master

  Future:
  Add a gesture pruning service to remove lowest rated gestures

  Used to wake the ESP32-S3 from deep sleep by moving it. Here are some steps 
  to fine-tune the sensitivity:

  a) Increase the Interrupt Threshold: This makes the accelerometer less 
     sensitive to small movements.
  b) Increase the Interrupt Duration: This ensures that only movements lasting 
     longer than a specified time will trigger an interrupt.
  c) Enable the High-Pass Filter: This filters out low-frequency signals 
     (such as gravity) and focuses on higher-frequency signals (such as sudden movements).

*/

#include "Accelerometer.h"

LIS3DH myIMU( I2C_MODE, 0x18 );

Accelerometer::Accelerometer(){}

void Accelerometer::begin()
{ 
  oldxPos = 120;
  oldyPos = 120;

  myIMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 16;        //Max G force readable.  Can be: 2, 4, 8, 16

  myIMU.settings.adcEnabled = 1;
  myIMU.settings.tempEnabled = 1;
  myIMU.settings.xAccelEnabled = 1;
  myIMU.settings.yAccelEnabled = 1;
  myIMU.settings.zAccelEnabled = 1;

  myIMU.applySettings();

  // Deep sleep awaken sensativity package

  uint8_t ctrlReg2;
  myIMU.readRegister(&ctrlReg2, LIS3DH_CTRL_REG2);
  ctrlReg2 |= 0x08; // Enable high-pass filter
  myIMU.writeRegister(LIS3DH_CTRL_REG2, ctrlReg2);

  myIMU.writeRegister(LIS3DH_INT1_THS, 16 );        // threshold is 1 - 127
  myIMU.writeRegister(LIS3DH_INT1_DURATION, 10 );   // interrupt duration 1 - 127 (increase to make it less sensitive)

  uint8_t dataToWrite = 0;

  //LIS3DH_INT1_CFG   
  //dataToWrite |= 0x80;//AOI, 0 = OR 1 = AND
  //dataToWrite |= 0x40;//6D, 0 = interrupt source, 1 = 6 direction source
  //Set these to enable individual axes of generation source (or direction)
  // -- high and low are used generically
  dataToWrite |= 0x20;//Z high
  //dataToWrite |= 0x10;//Z low
  dataToWrite |= 0x08;//Y high
  //dataToWrite |= 0x04;//Y low
  dataToWrite |= 0x02;//X high
  //dataToWrite |= 0x01;//X low
  myIMU.writeRegister(LIS3DH_INT2_CFG, dataToWrite);
  
  //LIS3DH_INT1_THS   
  dataToWrite = 0;
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x10; // 1/8 range
  myIMU.writeRegister(LIS3DH_INT2_THS, dataToWrite);
  
  //LIS3DH_INT1_DURATION  
  dataToWrite = 1;
  //minimum duration of the interrupt
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x01; // 1 * 1/50 s = 20ms
  myIMU.writeRegister(LIS3DH_INT2_DURATION, dataToWrite);
  
  //LIS3DH_CLICK_CFG   
  dataToWrite = 0;
  //Set these to enable individual axes of generation source (or direction)
  // -- set = 1 to enable
  //dataToWrite |= 0x20;//Z double-click
  dataToWrite |= 0x10;//Z click
  //dataToWrite |= 0x08;//Y double-click 
  dataToWrite |= 0x04;//Y click
  //dataToWrite |= 0x02;//X double-click
  dataToWrite |= 0x01;//X click
  myIMU.writeRegister(LIS3DH_CLICK_CFG, dataToWrite);
  
  //LIS3DH_CLICK_SRC
  dataToWrite = 0;
  //Set these to enable click behaviors (also read to check status)
  // -- set = 1 to enable
  //dataToWrite |= 0x20;//Enable double clicks
  dataToWrite |= 0x04;//Enable single clicks
  //dataToWrite |= 0x08;//sine (0 is positive, 1 is negative)
  dataToWrite |= 0x04;//Z click detect enabled
  dataToWrite |= 0x02;//Y click detect enabled
  dataToWrite |= 0x01;//X click detect enabled
  myIMU.writeRegister(LIS3DH_CLICK_SRC, dataToWrite);
  
  //LIS3DH_CLICK_THS   
  dataToWrite = 0;
  //This sets the threshold where the click detection process is activated.
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x0A; // ~1/16 range
  myIMU.writeRegister(LIS3DH_CLICK_THS, dataToWrite);
  
  //LIS3DH_TIME_LIMIT  
  dataToWrite = 0;
  //Time acceleration has to fall below threshold for a valid click.
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x08; // 8 * 1/50 s = 160ms
  myIMU.writeRegister(LIS3DH_TIME_LIMIT, dataToWrite);
  
  //LIS3DH_TIME_LATENCY
  dataToWrite = 0;
  //hold-off time before allowing detection after click event
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x08; // 4 * 1/50 s = 160ms
  myIMU.writeRegister(LIS3DH_TIME_LATENCY, dataToWrite);
  
  //LIS3DH_TIME_WINDOW 
  dataToWrite = 0;
  //hold-off time before allowing detection after click event
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x10; // 16 * 1/50 s = 320ms
  myIMU.writeRegister(LIS3DH_TIME_WINDOW, dataToWrite);

  //LIS3DH_CTRL_REG5
  //Int1 latch interrupt and 4D on  int1 (preserve fifo en)
  myIMU.readRegister(&dataToWrite, LIS3DH_CTRL_REG5);
  dataToWrite &= 0xF3; //Clear bits of interest
  dataToWrite |= 0x08; //Latch interrupt (Cleared by reading int1_src)
  //dataToWrite |= 0x04; //Pipe 4D detection from 6D recognition to int1?
  myIMU.writeRegister(LIS3DH_CTRL_REG5, dataToWrite);

  //LIS3DH_CTRL_REG3
  //Choose source for pin 1
  dataToWrite = 0;
  dataToWrite |= 0x80; //Click detect on pin 1
  //dataToWrite |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
  //dataToWrite |= 0x20; //AOI2 event ()
  //dataToWrite |= 0x10; //Data ready
  //dataToWrite |= 0x04; //FIFO watermark
  //dataToWrite |= 0x02; //FIFO overrun
  myIMU.writeRegister(LIS3DH_CTRL_REG3, dataToWrite);

  //LIS3DH_CTRL_REG6
  //Choose source for pin 2 and both pin output inversion state
  dataToWrite = 0;
  dataToWrite |= 0x80; //Click int on pin 2
  //dataToWrite |= 0x40; //Generator 1 interrupt on pin 2
  //dataToWrite |= 0x10; //boot status on pin 2
  //dataToWrite |= 0x02; //invert both outputs
  myIMU.writeRegister(LIS3DH_CTRL_REG6, dataToWrite);

  // Enable ESP32 to wake up on INT2 (GPIO13) high level
  esp_sleep_enable_ext1_wakeup(BIT(GPIO_NUM_14), ESP_EXT1_WAKEUP_ANY_HIGH);

  if (myIMU.begin() != 0) 
  {
    Serial.println("Could not start LIS3DH");
    video.stopOnError("Could not", "start", "accelerometer", "Stopping", " " ); 
  }

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if ( wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 ) 
  {
    // This is a warm start
    Serial.println("ESP32 woke up from deep sleep due to an external wake-up (movement detected).");
    return;
  } 
  else 
  {
    // This is a cold start
    Serial.println("ESP32 cold start");
  }

  angleTimer = millis();

  movetimer = millis();
  detecttimer = millis();
  recordtimer = millis();
  firsttime = true;
  olddtw = 0;
  recordi = 0;
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

  Serial.println( F("Accel gesture training mode on" ) );
  
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

float Accelerometer::getXaccelReading()
{
  return myIMU.readFloatAccelY();
}

// Get values from accelermoter, apply scale and filter

bool Accelerometer::getAccelValues()
{
  nx = myIMU.readFloatAccelX() * scaler ;
  ny = myIMU.readFloatAccelY() * scaler ;
  nz = myIMU.readFloatAccelZ() * scaler ;

  nx += 300;    // Force the readings into positive territory
  ny += 300;
  nz += 300;

  return true;
}

bool Accelerometer::isOutsidePercent( float oldposition, float newposition, float perc )
{
  float lowerBound = oldposition * ( 1 - perc );
  float upperBound = oldposition * ( 1 + perc );
  return (newposition < lowerBound || newposition > upperBound);
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

  if ( isOutsidePercent( nx3, nx, tollerance ) )
  {
    /*
    Serial.print( "X " );
    Serial.print( nx );
    Serial.print( ", " );
    Serial.println( nx3 );
    */
    
    firsttime = true;  
    movetimer = millis();
    return true;
  }

  if ( isOutsidePercent( ny3, ny, tollerance ) )
  {
    /*
    Serial.print( "Y " );
    Serial.print( ny );
    Serial.print( ", " );
    Serial.println( ny3 );
    */

    firsttime = true;  
    movetimer = millis();
    return true;
  }

   if ( isOutsidePercent( nz3, nz, tollerance ) )
   {
    /*
    Serial.print( "Z " );
    Serial.print( nz );
    Serial.print( ", " );
    Serial.println( nz3 );
    */

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

float Accelerometer::getAngle()
{
  float accel_x = myIMU.readFloatAccelX();  // Read X-axis value
  float accel_y = myIMU.readFloatAccelY();  // Read Y-axis value

  return atan2(accel_y, accel_x) * 180 / PI;
}


void Accelerometer::loop()
{

// Experimenting with accel, plotting white ball around perimeter of screen

  if ( millis() - angleTimer > 500 )
  {
    angleTimer = millis();
 
    xAccel = myIMU.readFloatAccelX();
    yAccel = myIMU.readFloatAccelY();

    Serial.print( "Accel x = " );
    Serial.print( xAccel, 4 );
    Serial.print( " Accel y = " );
    Serial.println( yAccel, 4 );

    
    float rad = atan2( yAccel, xAccel );  
    float degree = rad * 180.0 / PI;  // Convert radians to degrees
    
    // Make sure the angle is between 0 and 360 degrees
    if (degree < 0) degree += 360;
    
    angle = degree;
    
    int radius = 120;  // Radius of the circle (half of the 240x240 display size)
    int xPos = 120 + radius * cos(radians(angle));  // Convert angle to radians for math
    int yPos = 120 + radius * sin(radians(angle));
    
    oldxPos = xPos;
    oldyPos = yPos;

    // Draw a 20-pixel circle at the calculated position

    gfx->fillCircle(oldxPos, oldyPos, 15, BLACK);  // Use 10 to make it a 20px diameter

    gfx->fillCircle(xPos, yPos, 10, BLUE);  // Use 10 to make it a 20px diameter

  }




  // Training mode, records gesture templates, stores to .ages file (Accelerometer Gesture Template)

  // Record multiple templates, then stop

  if ( ( trainingMode ) && ( ! recordingTemplate ) ) 
  {
    if ( ! firstnotice )
    {
      firstnotice = true;

      String mef = "Template recording, ";
      mef += String( gesturecount );
      mef += ", ";

      if ( typecounter == 0 ) { mef += type1; }
      if ( typecounter == 1 ) { mef += type2; }
      if ( typecounter == 2 ) { mef += type3; }
      if ( typecounter == 3 ) { mef += type4; }

      logger.info( mef );
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
        logger.info("Make a gesture... ");
        firstnotice = true;
      }

      if ( detectStartOfGesture() )
      {
        logger.info("Sensing gesture");
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
            logger.info( "Accellerometer gesture detection inconclusive" );
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
