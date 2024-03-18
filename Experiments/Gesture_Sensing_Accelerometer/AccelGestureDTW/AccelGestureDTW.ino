/*
Reflections, mobile entertainment platform

Experiment to recognizes gestures by moving the Reflections board 
(LIS3DHTR 3-Axis Accelerometer)

Supports multiple gestures and stores them on the SD/NAND
Each gesture is 1.5 seconds, length is adjustable in the code

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Depends on:
https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR

Modified from Federico Terzi experiment at
https://github.com/CieloStrive/GESTURE-RECOGNITION-DYNAMIC-TIME-WARPING/tree/master

*/

#include <Wire.h>
#include "LIS3DHTR.h"
#include "config.h"
#include "sdios.h"
#include "SD.h"
#include "SPI.h"

LIS3DHTR<TwoWire> LIS;

#define tollerance 1.05
#define windowtime 1000
#define scaler 200
#define scalemax 1000
#define scalemin -1000

unsigned long movetimer;
bool firsttime = true;
int movecount = 1;

float nx0 = 1, nx1 = 1, nx2 = 1, nx3 = 1, nx4 = 1;
float ny0 = 1, ny1 = 1, ny2 = 1, ny3 = 1, ny4 = 1;
float nz0 = 1, nz1 = 1, nz2 = 1, nz3 = 1, nz4 = 1;
float nx, ny, nz;

ArduinoOutStream cout(Serial);   // Serial output stream

long timer = 0;

int8_t state = 0;
int8_t co = 0;

#define framecount 50
#define framedelay 40

float accx[2][ framecount ];
float accy[2][ framecount ];
float accz[2][ framecount ];

float DTW[ framecount ][ framecount ];

float dx = 0;
float dy = 0;
float dz = 0;

float threshold = 0;

boolean SDMounted = false;

char typedFileName[32];
char fileName[32];
char *ext = ".ages";

void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}

void getNameAndSave()
{
  if (Serial.available() > 0)
  {
    int ix = 0; // Character counter
    int ix1 = 0; // Loop turn off
    while (ix1 == 0) 
    {
      while ( Serial.available() > 0 ) 
      {
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
        SD.remove( fileName ); // Don't append to an existing file

        // Create the .ages file
        File file = SD.open( fileName, FILE_WRITE );
        if ( !file )
          {
           Serial.print( "Error opening file for write: " );
           Serial.println( fileName );
           return;
         }

        // Save gesture to .ages file

        // Incomplete, todo
            
        Serial.println( F( "File saved" ) ); 
      }
    }
  }
}

/* 
 * Record movement
 * From the original code:
 * j = 0 = tr_accx[] template record
 * j = 1 = te_accs[] comparison record
*/

void recorder( int j )
{
  for (int i = 0 ; i < framecount ; i++)
  {
    delay( framedelay ); 
    accx[j][i] = LIS.getAccelerationX() * scaler;
    accy[j][i] = LIS.getAccelerationY() * scaler; 
    accz[j][i] = LIS.getAccelerationZ() * scaler; 
  }
}

// Start DTW threshold processing

float DTW_THRESHOLD()
{
  float scale = 0.5;
  float dir = 0; 

  for ( int8_t i = 0; i < framecount ; i++ )
  {
    if (i == 0)
    {
      dx = accx[0][i] - accx[1][i];
      dy = accy[0][i] - accy[1][i];
      dz = accz[0][i] - accz[1][i];
 
      dir = ( accx[0][i] * accx[1][i] + accy[0][i] * accy[1][i] + accz[0][i] * accz[1][i] )
        / ( DTWNORM( accx[0][i], accy[0][i], accz[0][i]) * DTWNORM( accx[1][i], accy[1][i], accz[1][0] ) + 0.0000001 );
 
      DTW[i][i] = ( 1-scale*dir ) * DTWNORM( dx,dy,dz );
    }
    else
    {
      dx = accx[0][i] - accx[1][0];
      dy = accy[0][i] - accy[1][0];
      dz = accz[0][i] - accz[1][0];
 
      dir = ( accx[0][i] * accx[1][0] + accy[0][i] * accy[1][0] + accz[0][i] * accz[1][0])
        / ( DTWNORM( accx[0][i], accy[0][i], accz[0][i]) * DTWNORM( accx[1][0], accy[1][0], accz[1][0] ) + 0.0000001);
 
      DTW[i][0] = ( 1-scale*dir ) * DTWNORM( dx, dy, dz ) + DTW[i-1][0];

      dx = accx[0][0] - accx[1][i];
      dy = accy[0][0] - accy[1][i];
      dz = accz[0][0] - accz[1][i];
 
      dir = ( accx[0][0] * accx[1][i] + accy[0][0] * accy[1][i] + accz[0][0] * accz[1][i] )
        / ( DTWNORM( accx[0][0], accy[0][0], accz[0][0]) * DTWNORM( accx[1][i], accy[1][i], accz[0][i] ) + 0.0000001);
 
      DTW[0][i] = ( 1-scale * dir ) * DTWNORM( dx, dy, dz ) + DTW[0][i-1];
    }
  }

  for ( int8_t i = 1 ; i < framecount ; i++)
  {
    for ( int8_t j = 1 ; j < framecount ; j++)
    {
      dx = accx[0][i] - accx[1][j];
      dy = accy[0][i] - accy[1][j];
      dz = accz[0][i] - accz[1][j];
        
      dir = ( accx[0][i] * accx[1][j] + accy[0][i] * accy[1][j] + accz[0][i]*accz[1][j])
        / ( DTWNORM(accx[0][i], accy[0][i], accz[0][i]) * DTWNORM( accx[1][j], accy[1][j], accz[1][j] ) + 0.0000001);  

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

float DTWMIN( float &a, float &b ,float &c )
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

float DTWNORM( float dx, float dy, float dz){
  return sqrt(dx*dx + dy*dy + dz*dz);
}

// Get values from accelermoter, apply scale and filter

bool getAccelValues()
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

// Returns true when accelerator is moving

bool detectStartOfGesture()
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

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  long time = millis();
  while (!Serial && ( millis() < time + 5000) ); // wait up to 5 seconds for Arduino Serial Monitor  
  delay(2000);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("GestureDTW Experiment, Reflections project by Frank Cohen fcohen@starlingwatch.com");  

  // Set-up the Reflections board hardware and buses

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, LOW);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);  

  // NAND is an SMT SD storage device

  pinMode( NAND_SPI_CS, OUTPUT );
  digitalWrite( NAND_SPI_CS, LOW);

  if ( ! SD.begin( NAND_SPI_CS ) )
  {
    Serial.println(F("SD card failed"));
    SDMounted = false;
  }
  else
  {
    Serial.println(F("SD card mounted"));
    SDMounted = true;
  }   

  Wire.begin( I2CSDA, I2CSCL );

  LIS.begin( Wire );  
  LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
  LIS.setHighSolution(true); //High solution enable

  movetimer = millis();
  firsttime = true;
  movecount = 1;

  /* Training the Human
  while(1)
  {

    Serial.println("TEMPLATE RECORD START: record 1.5 seconds");
    while ( ! detectStartOfGesture() ) { delay( 500 ); }
    recorder( 1 );
    delay(2000);
  
    Serial.println("COMPARISON RECORD START: record 1.5 seconds");
    while ( ! detectStartOfGesture() ) { delay( 500 ); }
    recorder( 0 );

    threshold = DTW_THRESHOLD();
    Serial.println( threshold );

    delay(2000);
  }
  */  
 
  float olddtw = 0;

  while( 1 )
  {
    Serial.println("TEMPLATE RECORD START: record 1.5 seconds");
    while ( ! detectStartOfGesture() ) { delay( 500 ); }
    recorder( 1 );
    delay(2000);
    
    olddtw = DTW_THRESHOLD();

    for ( int i = 0; i<3; i++ )
    {
      Serial.print("Make the gesture");
      while ( ! detectStartOfGesture() ) { delay( 500 ); }
      Serial.println("-");
      recorder( 0 );

      float newdtw = DTW_THRESHOLD();

      Serial.print( newdtw );
      Serial.print( "\t" );
      Serial.print( olddtw );
      Serial.print( "\t" );
      Serial.println( 100 * ( olddtw / newdtw ) );

      float ratperc =100 * ( olddtw / newdtw );
      if ( ( ratperc > 0 ) &&
           ( ( ratperc < 115 ) && ( ratperc > 85 ) )
          )



      {
        Serial.print(" Recognized");
        Serial.print( "\t" );
        Serial.println( ratperc );
      }

  /*
      if ( ( ( rating >= 0 ) && ( rating <= threshold + 5) ) )
      {
        Serial.println(" Recognized");
      }
  */

      delay(2000);
    }
  }
}

void loop(){
  delay(500);
}
