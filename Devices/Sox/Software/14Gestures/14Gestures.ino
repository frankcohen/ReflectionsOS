/*
Gesture Learning Engine (GLE)
by Frank Cohen
License: GPL v3

Read an 8x8 array of distances from the VL53L5CX
Store a gesture file
Identify % similarity between gestures
Straight similarity, with no capability of rotation around the array, not yet

Learn a gesture.
- Save readings to gesture (.ges) file, then rename the file manually

Detect a gesture
- Observe TOF sensor
- Show % detected for each gesture
- Repeat

Depends on SparkFun Electronics VL53L5CX library

*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h> // http://librarymanager/All#SparkFun_VL53L5CX

char keypress;
int knownGestureCount = 0;
uint16_t *knownGestures;
uint16_t *oneFrame;
char *gestureNames;
char typedFileName[32];
char fileName[32];
char *ext = ".ges";

SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // Result data class structure, 1356 byes of RAM

int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output

#define gestureSize 64
#define gestureWidth 8
#define gestureHeight 8
#define maxGestures 16
#define nameSize 50
#define variance 10
#define threashold 32   // 50% of 64 = 32

// Prints a gesture to the serial monitor

void showGesture( int gesturenum )
{
  // Display the name
  Serial.println( (char *) gestureNames + ( gesturenum * nameSize ) );

  for (int y = 0; y < gestureHeight; y++ )
  {
    for (int x = 0; x < gestureWidth; x++ )
    {
      Serial.print("\t");

      uint16_t * mep = &knownGestures[ ( y * gestureHeight ) + ( gesturenum * gestureSize ) ];
      Serial.print( mep[x] );
    }
    Serial.println("");
  }
  Serial.println("");
}

void showGestureOneFrame()
{
  // Display the name
  Serial.println( "OneFrame:" );

  for (int y = 0; y < gestureHeight; y++ )
  {
    for (int x = 0; x < gestureWidth; x++ )
    {
      Serial.print("\t");

      uint16_t * mep = oneFrame + x + ( y * gestureWidth );
      Serial.print( mep[0] );
    }
    Serial.println("");
  }
  Serial.println("");
}

// Prints current sensor readings to the serial monitor

void getLiveToOneFrame()
{
  int oney = 0;

  if (myImager.isDataReady() == true)
  {
    if (myImager.getRangingData(&measurementData)) //Read distance data into array
    {
      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = gestureWidth * (gestureWidth -  1) ; y >= 0 ; y -= gestureWidth )
      {
        for (int x = 0; x < gestureWidth; x++ )
        {
          //Serial.print("\t");
          //Serial.print(measurementData.distance_mm[x + y]);

          uint16_t * mep = oneFrame + x + ( oney * gestureWidth );
          mep[0] = measurementData.distance_mm[ x + y ];
        }
        //Serial.println();
        oney++;
      }
      //Serial.println();
    }
  }
}

// Prints a gesture stored on the SD to the serial monitor

void showGestureSD( String gfName )
{
  File file = SD.open( gfName, FILE_READ );

  if ( !file )
  {
    Serial.println("Error opening file for write. showGestureSD.");
    return;
  }

  // Display the contents
  for ( int i = 0; i++; i < gestureHeight )
  {
    Serial.print( i );
    Serial.print( ": " );

    // Display the contents
    for ( int j = 0; j++; j < gestureWidth )
    {
      byte val[2] = {0,0};
      int res = file.read( val, 1);
      if ( !res )
      {
        Serial.println("");
        Serial.print("Error while reading file ");
        Serial.println( file.name() );
      }
      Serial.print( res );
    }

    Serial.println( "" );
  }
}

boolean saveGestureToSD()
{
  Serial.print("saveGestureToSD fileName = ");
  Serial.println( fileName );

  if ( ( myImager.isDataReady() == false ) && (myImager.getRangingData(&measurementData)) )
  {
    Serial.print( "Sensor data not ready" );
    return false;
  }

  SD.remove( fileName ); // Don't append to an existing file

  File file = SD.open( fileName, FILE_WRITE );
  if ( !file )
  {
    Serial.print( "Error opening file for write: " );
    Serial.println( fileName );
    return false;
  }

  for (int y = imageWidth * (imageWidth -  1) ; y >= 0 ; y -= imageWidth)
  {
    for (int x = 0; x<= imageWidth - 1 ; x++ )
    {
      uint16_t vz = measurementData.distance_mm[ x + y ];
      uint8_t xlow = vz & 0xff;
      uint8_t xhigh = (vz >> 8);
      file.write( xhigh );
      file.write( xlow );
    }
  }

  file.close();
  Serial.println( "saveGestureToSD complete" );
  return true;
}

void loadAllGesturesFromSD()
{
  boolean more = true;
  knownGestureCount = 0;
  File sdit = SD.open("/");

  while ( more )
  {
    File file = sdit.openNextFile();
    if ( ! file )
    {
      more = false;
      sdit.close();
      return;
    }
    String mef = file.name();
    if ( mef.endsWith(".ges") )
    {
      Serial.print( "Loading gesture ");
      Serial.println( knownGestureCount );

      uint16_t * kges = knownGestures + ( knownGestureCount * gestureSize );
      for ( int i=0; i < gestureSize; i++ )
      {
        int rv = file.read();
        int rv2 = file.read();
        kges[i] = ( rv * 256 ) + rv2;
      }

      String nm = file.name();
      nm.toCharArray( (char *) gestureNames + ( knownGestureCount * nameSize ), nameSize );

      knownGestureCount++;
    }
  }

  sdit.close();
  return;
}

int compareGestures( uint16_t *gesture1, uint16_t *gesture2 )
{
  int rating = 0;

  for ( int i = 0; i < gestureHeight; i++ )
  {
    for ( int j = 0; j < gestureWidth; j++ )
    {
      int cellval = gesture1[ (i * gestureWidth ) + j ];
      int matchval = gesture2[ (i * gestureWidth ) + j ];

      /*
       Serial.print( cellval );
       Serial.print( "," );
       Serial.print( matchval );
       Serial.print(",");
      */

      if ( ( cellval - variance > 0 ) && ( cellval > variance ) )
      {
        cellval = cellval - variance;
      }

      if ( ( matchval > cellval ) && ( matchval < cellval + ( 2 * variance ) ) )
      {
        rating++;
      }
      /*
      Serial.print( rating );
      Serial.print( "," );
      */
    }
  }
  return rating;
}

void showStats()
{
  //Serial.println( "getLiveToOneFrame()" );
  getLiveToOneFrame();

  //Serial.println( "showGesture(0)" );
  //showGesture(0);

  //Serial.print( "knownGestureCount = ");
  //Serial.println( knownGestureCount );

  for ( int i = 0; i<knownGestureCount; i++ )
  {
    int score = compareGestures( oneFrame, knownGestures + ( i * gestureSize ) );

    Serial.print( (char *) gestureNames + ( i * nameSize ) );
    Serial.print( " " );
    Serial.print( score );
    Serial.print( " " );

    if ( score >= threashold )
    {
      Serial.println( "HIT" );
    }
    else
    {
      Serial.println( "" );
    }
  }
  Serial.println( " " );
}

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("");
  Serial.println( "Gesture Learning Engine (GLE)" );
  Serial.println( "" );
  Serial.println( "Enter gesture name to save to file" );

  Wire.begin(21,22); //This resets to 100kHz I2C
  Wire.setClock(400000); //Sensor has max I2C freq of 400kHz

  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");
  if (myImager.begin() == false)
  {
    Serial.println(F("Sensor not found"));
    while (1) ;
  }

  myImager.setResolution( 64 ); //Enable all 8 x 8 for 64 pads
  imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width
  myImager.startRanging();

  if ( ! SD.begin( 4 ) )
  {
    Serial.println(F("SD card failed"));
    while(1);
  }
  else
  {
    Serial.println(F("SD card mounted"));
  }

  knownGestures = (uint16_t *) malloc( gestureSize * maxGestures * 2 );
  if ( knownGestures == NULL )
  {
    Serial.println(F("knownGestures malloc failed"));
    while(1);
  }

  memset( knownGestures, 0, gestureSize * maxGestures * 2 );

  gestureNames = (char *) malloc( nameSize * maxGestures );
  if ( gestureNames == NULL )
  {
    Serial.println(F("gestureNames malloc failed"));
    while(1);
  }

  oneFrame = (uint16_t *) malloc( gestureSize * 2 );
  if ( oneFrame == NULL )
  {
    Serial.println(F("oneFrame malloc failed"));
    while(1);
  }

  memset( oneFrame, 0, gestureSize );

  loadAllGesturesFromSD();
}

void loop()
{
  if (Serial.available() > 0)
  {
    int ix = 0; // Character counter
    int ix1 = 0; // Loop turn off
    while (ix1 == 0) {
      while ( Serial.available() > 0 ) {
        char recieved = Serial.read();
        if(recieved != '\n' && recieved != '\r')
        {
          typedFileName[ix] = recieved;
          ix++;
          delay(25); // Need for stability.
        }
      }
      if (ix != 0 )
      {
        typedFileName[ix] = '\0';
        ix1 = 1;

        strcpy( fileName, "/" );
        strcat( fileName, typedFileName );
        strcat( fileName, ext );

        saveGestureToSD();
      }
    }
  }

  showStats();
  delay(1000);
}
