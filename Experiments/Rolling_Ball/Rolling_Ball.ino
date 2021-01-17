/*
 * Ball rolls around the display
 * Moves by tilting the board
 * 
 * Wiring, repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * 
 * Reflections project: A wrist watch
 * 
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * January 17, 2021

 * Using Arduino_GFX class by @moononournation
 * https://github.com/moononournation/Arduino_GFX
 * For speed and support of MPEG video
 * See Display_Logo sketch for video player
 */

#include <Wire.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Arduino_GFX_Library.h>

#define displaywidth 240
#define displayheight 240

float ballX = displaywidth / 2;
float ballY = displayheight / 2;
int ballSize = 20;
float oldballX = ballX;
float oldballY = ballY;
float accelerationX = 0;
float accelerationY = 0;
float frictionX = 0;
float frictionY = 0;
const float maxVelocity = 1.5;
float startX = 0;
float startY = 0;
long velocityX = 0;
long velocityY = 0;

long previousTimestamp = 0;

long previousMillis = 0;
long currentMillis = 0;

// Definitions

#define SCK 18
#define MOSI 23
#define MISO 19
#define SS 4

#define DisplayCS 32     //TFT display on Adafruit's ST7789 breakout board
#define DisplaySDCS 4    //SD card on the Adafruit ST7789 breakout board
#define DisplayRST 17    //Reset for Adafruit's ST7789 breakout board
#define DisplayDC 16     //DC for Adafruit's ST7789 breakout board
#define TFT_BRIGHTNESS 128
#define TFT_BL 33        // Just so it doesn't conflict with something else

// ST7789 Display
Arduino_HWSPI *bus = new Arduino_HWSPI(DisplayDC /* DC */, DisplayCS /* CS */, SCK, MOSI, MISO);
Arduino_ST7789 *gfx = new Arduino_ST7789(bus, -1 /* RST */, 2 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);

// BNO055 Accelerometer, Compass, Gyro Breakout Board
// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 gyro = Adafruit_BNO055(55, 0x28);

boolean readytostart = true;

void enableOneSPI( int deviceCS )
{
  if ( deviceCS != DisplayCS ) { disableSPIDevice( DisplayCS ); }
  if ( deviceCS != DisplaySDCS ) { disableSPIDevice( DisplaySDCS ); }
  if ( deviceCS != DisplayRST ) { disableSPIDevice( DisplayRST ); }
  if ( deviceCS != DisplayDC ) { disableSPIDevice( DisplayDC ); }
  Serial.print( F("Enabling SPI device on pin ") );
  Serial.println( deviceCS );
  pinMode(deviceCS, OUTPUT);
  digitalWrite(deviceCS, LOW);
}

void disableSPIDevice( int deviceCS )
{
    Serial.print( F("Disabling SPI device on pin ") );
    Serial.println( deviceCS );
    pinMode(deviceCS, OUTPUT);
    digitalWrite(deviceCS, HIGH);
}

/*
 * Set-up display components
 */

boolean setupDisplay()
{
  enableOneSPI( DisplayCS );
  gfx->begin();
  gfx->fillScreen(YELLOW);
  return true;  
}

boolean setupTiltOMatic()
{
  if ( !gyro.begin() )
  {
    Serial.println( "BNO055 compass, accelerometer, gyroscope did not start" );
    return false;    
  }
  /* Use external crystal for better accuracy */
  gyro.setExtCrystalUse(true);

  return true;  
  
}

boolean setupSound()
{
  return true;
}

// Returns the value limited to the limit value

float minmax( float value, float limit )
{
  return _max( _min( value, limit ), -limit);
}

// Decreases the absolute value of a number but keeps it's sign, doesn't go below abs 0

float slow( float number, float difference )
{
  if ( abs(number) <= difference ) return 0;
  if (number > difference) return number - difference;
  return number + difference; 
}

// sign function
// if x>0 return 1
// if x<0 return -1
// if x=0 return 0
// Thanks to AlphaBeta on the Arduino support forums for this.

template <typename type>
type sign(type value) {
 return type((value>0)-(value<0));
}

/*
 * Ball movement with speed
 * Thanks to Hunor Marton Borbel https://www.youtube.com/watch?v=bTk6dcAckuI for showing
 * me how to build this movement + friction algoritm.
 * TODO: Add velocity and friction
 */
 
void moveBall( float ytiltloc, float ztiltloc )
{
  //if ( ( abs( ytiltloc ) > 5 ) || ( abs( ztiltloc ) > 5 ) ) { return; }

  // Negative ytiltloc means tilting to the left
  // Positive means tilting to the right

  // Negative ztiltloc means tilting away from you
  // Positive means tilting towards you

  const float mouseDeltaX = - minmax( startX - ytiltloc, 15);
  const float mouseDeltaY = - minmax( startY - ztiltloc, 15);

  const float rotationY = mouseDeltaX * 0.8; // Max rotation = 12
  const float rotationX = mouseDeltaY * 0.8;

  const float gravity = 2;
  const float friction = 0.01; // Coefficients of friction

  accelerationX = gravity * sin((rotationY / 180) * PI);
  accelerationY = gravity * sin((rotationX / 180) * PI);
  frictionX = gravity * cos((rotationY / 180) * PI) * friction;
  frictionY = gravity * cos((rotationX / 180) * PI) * friction;

  const float maxVelocity = 1.5;
  // Time passed since last cycle divided by 16
  // This function gets called every 16 ms on average so dividing by 16 will result in 1
  const long timeElapsed = ( millis() - previousTimestamp) / 16;

  const float velocityChangeX = accelerationX * timeElapsed;
  const float velocityChangeY = accelerationY * timeElapsed;
  const float frictionDeltaX = frictionX * timeElapsed;
  const float frictionDeltaY = frictionY * timeElapsed;
  
  if (velocityChangeX == 0) {
    // No rotation, the plane is flat
    // On flat surface friction can only slow down, but not reverse movement
    velocityX = slow(velocityX, frictionDeltaX);
  } else {
    velocityX = velocityX + velocityChangeX;
    velocityX = max( _min( velocityX, 1.5 ), -1.5 );
    velocityX = velocityX - sign( velocityChangeX ) * frictionDeltaX;
    velocityX = minmax( velocityX, maxVelocity );
  }

  if (velocityChangeY == 0) {
    // No rotation, the plane is flat
    // On flat surface friction can only slow down, but not reverse movement
    velocityY = slow( velocityY, frictionDeltaY );
  } else {
    velocityY = velocityY + velocityChangeY;
    velocityY =
      velocityY - sign(velocityChangeY) * frictionDeltaY;
    velocityY = minmax( velocityY, maxVelocity );
  }

  // Are we on the boundary? if so stop

  const float nextX = ballX + velocityX;
  const float nextY = ballY + velocityY;
  if ( nextX > ( displaywidth - ( ballSize * 2 ) ) ) { velocityX = 0; }
  if ( nextX <  0 ) { velocityX = 0; }
  if ( nextY> ( displayheight - ( ballSize * 2) ) ) { velocityY = 0; }
  if ( nextY < 0 ) { velocityY = 0; }

  /*
  Serial.print( "ytiltloc = " );
  Serial.print( ytiltloc );
  Serial.print( " ztiltloc = " );
  Serial.print( ztiltloc );
  Serial.print( " nextX = " );
  Serial.print( nextX );  
  Serial.print( " nextY = " );
  Serial.print( nextY );  
  Serial.print( " ballX = " );
  Serial.print( ballX );  
  Serial.print( " ballY = " );
  Serial.println( ballY );  
  */
  
  ballX += velocityX;
  ballY += velocityY;
}

void drawBall()
{
  if ( ! ( ( ballX == oldballX ) && ( ballY == oldballY ) ) )
  {
    gfx->fillCircle(ballX + ballSize, ballY + ballSize, ballSize + 5, YELLOW);
    gfx->fillCircle(oldballX + ballSize, oldballY + ballSize, ballSize, BLUE);
  }
  
  oldballX = ballX;
  oldballY = ballY;
}

void smartdelay( long interval )
{
  unsigned long start = millis();
  do
  {
    // Background tasks, if any, go here
  } while (millis() - start < interval);
}

/* ----------------------SETUP-------------------- */

void setup() {
  Serial.begin(115200);

  readytostart = true; 

  if ( !setupDisplay() )
  {
    Serial.println( "setupDisplay failed" );
    readytostart = false;
  }

  if ( !setupTiltOMatic() )
  {
    readytostart = false;
  }

  if ( !setupSound() )
  {
    readytostart = false;
  }
}

/* ----------------------LOOP-------------------- */

void loop() 
{
  if ( !readytostart ) { return; }

  /* Get a new gyro sensor event */
  sensors_event_t event;
  gyro.getEvent(&event);
    
  moveBall( (float) event.orientation.y, (float) event.orientation.z * -1 );

  drawBall();

  gfx->setTextColor(WHITE, RED);
  int number = random(25555);
  int intPart = number/100;
  int decimalPart = number - 100*intPart;
  // sprintf(TX,"%03d.%02d", intPart, decimalPart); //  XXX.XX
  gfx->setTextSize(2);    
  gfx->setCursor(50, 30);  
  gfx->print(TX);  
  
  gfx->setTextSize(3);    
  gfx->setCursor(30, 65);  
  gfx->print(TX);    

  smartdelay( 20 );
  
}
