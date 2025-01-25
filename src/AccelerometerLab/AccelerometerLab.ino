/*

AccelerometerLab 
by Frank Cohen
fcohen@starlingwatch.com

LIS3DH is an inexpensive 3 Axis accelerometer component. It senses movement
and acceleration from gravity. I use it to sense gestures in the
Reflections open-source mobile entertainment platform for visual controls
and to wake the ESP32-S3 processor from deep sleep. It senses movement, 
taps, clicks, shaking, and acceleration. Many libraries exist and 
generative AI such as ChatGPT is familiar and can generate code for
the sensor. I created this library to bring the examples together and 
provide answers I did not find elsewhere. For example, what are the best
sensitivity and gravity values to use?

I publish this library under an open-source license hoping you will contribute
your LIS3DH experiences, tips, and techniques for others to enjoy.

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Depends on these libraries:
Adafruit LIS3DH library, https://github.com/adafruit/Adafruit_LIS3DH
Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
PeakDetection, peak and valley detection, https://github.com/leandcesar/PeakDetection

Written in support of the Reflections project at:
https://github.com/frankcohen/ReflectionsOS

Datasheet comes with this source code, see:
1811031937_STMicroelectronics-LIS3DHTR_C15134 Accelerometer.pdf

Learn how to wake an ESP32-S3 from deep sleep with interrupt movement sensing at 
https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Deep%20Sleep/Deep_Sleep.md

Sensor and support library details:

getClick() bit definitions:
Bit	Name	   Description
6	  IA	     Interrupt Active: Set to 1 when a click event is detected.
5	  DCLICK	 Double Click: Set to 1 if a double click event is detected.
4	  SCLICK	 Single Click: Set to 1 if a single click event is detected.
3	  Sign	   Sign of Acceleration: Indicates the direction of the click event.
2	  Z	Z-Axis Contribution: Set to 1 if the Z-axis contributed to the click.
1	  Y	Y-Axis Contribution: Set to 1 if the Y-axis contributed to the click.
0	  X	X-Axis Contribution: Set to 1 if the X-axis contributed to the click.

Scaling factor for different ranges:
For LIS3DH_RANGE_2_G (±2g), the scaling factor is 0.001 g per count.
For LIS3DH_RANGE_4_G (±4g), the scaling factor is 0.002 g per count.
For LIS3DH_RANGE_8_G (±8g), the scaling factor is 0.004 g per count.
For LIS3DH_RANGE_16_G (±16g), the scaling factor is 0.008 g per count.

Data rate for different applications:
Low-speed movements (walking): 10–50 Hz., 100 ms delay between reads
Medium-speed movements (hand gestures): 50–100 Hz., 10 ms delay between reads
High-speed movements (vibrations, impacts): 200–400 Hz., 5 ms or less between reads

I used this prompt with ChatGPT v4, included here as an example:

Time Adjustment algorithm implemented in C and C++ as an Arduino 2.3 sketch to help me 
with an LIS3DH accelerometer. You helped me with in the past. This is a new approach. 
So do not use the previous approach. The input to this algorithms is a measurement from 
the accelerometer. The input will always be within the range of 1 to 30,000, always positive. 
The first input is always 15,000. Measurements are taken every 300 milliseconds. The first 
input establishes a Neutral Zone. This zone begins 10% below and ends 10% above the 
measurement. Subsequent measurements that fall inside the Neutral Zone make no change 
to the time. Immediately below the Neutral Zone is an Increase Zone. Subsequent measurements 
in the Increase Zone increase by 1 the current Time value. When the Time value reaches 13 is 
loops back to 1. When the Time value changes the code waits 500 milliseconds before the next 
subsequent measurement. Immediately above the Neutral Zone is a Decrease Zone. Subsequent 
measurements in the Decrease Zone decrease by 1 the current Time value. When the Time value 
reaches 0 is loops back to 12. When the Time value changes the code waits 500 milliseconds 
before the next subsequent measurement. If the Subsequent measurement is in the Decrease Zone 
after previously being in the Increase Zone, the no decrease of time is made and the location 
of the Neutral Zone recalculates to the new measurement. If the Subsequent measurement is in 
the Increase Zone after previously being in the Decrease Zone, the no increase of time is made 
and the location of the Neutral Zone recalculates to the new measurement. 

Depends on Esspresif ESP32 libraries at
[https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)

Using Arduino IDE 2.x, use Tools menu options:

Board: ESP32S3 Dev Module
CPU Frequency: 240Mhz (Wifi)
USB CDC On Boot: Enabled
Core Debug Level: Error
USB DFU On Boot: Disabled
Erase All Flash Before Sketch Upload: Disabled
Events Run On: Core 1
Flash Mode: QIO 80 Mhz
Flash Size: 8 MB (64MB)
JTAG Adapter: Integrated USB JTAG
Arduino Runs On: Core 1
USB Firmware MSC On Boot: Disabled
Partition Scheme: Custom, Reflections App (8MB OTA No SPIFFS)    See below
PSRAM: Disabled
Upload Mode: UART0/Hardware CDC
Upload Speed 921600
USB Mode: Hardware CDC and JTAG
Zigbee Mode: Disabled

Reflections uses a custom partition scheme to maximize available flash storage.
See instructions at: https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/Partition%20tables%20and%20optimizing%20memory%20in%20Arduino%20IDE.md
Arduino IDE 2.x automatically uses partitions.csv in the source code directory

PeakDetection library details:

Calculates Jerk value as the rate of change of acceleration (Δa/Δt), where 𝑎 is in physical units. 

Decrease lag for quicker responsiveness.
Default is 13

Increase threshold to reduce false positives
threshold = 2: Higher sensitivity (detects smaller deviations as peaks)
threshold = 3: Moderate sensitivity
threshold = 4: Lower sensitivity (detects only large deviations as peaks)
Default is 3

Adjust influence to balance the impact of peaks:

0.2 Low influence
Monitoring heart rate over time. Outlier values (e.g., due to noise or movement) should not significantly impact the overall trend.
Stable and Smooth Data: Use a lower influence value (e.g., 0.2 to 0.4) to emphasize trend stability.
Peaks have minimal effect on the trend.
Useful for data with noise or when the underlying trend is more important than transient peaks.
Risk: True peaks might not be detected effectively in quickly changing data.

0.5 Moderate influence
Tracking general motion patterns with an accelerometer. A balance between adapting to sudden movements and preserving the trend.
General Use: A moderate value around 0.5 works well in most cases.
Provides a balanced approach.
Peaks are considered, but the algorithm resists being overly affected by transient changes.
This is often a good starting point for most applications.

0.8 High influence
Imagine a real-time stock price where rapid changes need to be tracked.
Peaks (e.g., sudden price spikes) are quickly incorporated into the trend.
Dynamic and Noisy Data: Start with a higher influence value (e.g., 0.7 or higher).
Peaks are quickly incorporated into the trend.
Useful for highly dynamic data where rapid adaptation is necessary.
Risk: Noise or outliers may cause false positives or disrupt the trend.
Low Influence (influence ≈ 0.0):

The new data[j] is computed as a weighted combination of the current sample (newSample) and the previous trend (data[i]).
The influence parameter controls this weighting:
influence * newSample: Contribution from the detected peak.
(1.0 - influence) * data[i]: Contribution from the previous trend.

*/

#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <PeakDetection.h>

// Default settings

// R - Range default, 2, 4, 8, 16
#define range1 LIS3DH_RANGE_8_G

// Data rate
// Low-speed movements (walking): 10–50 Hz.
// Medium-speed movements (hand gestures): 50–100 Hz.
// High-speed movements (vibrations, impacts): 200–400 Hz.
#define datarate LIS3DH_DATARATE_100_HZ

// G - Lag
#define peaklag1 5

// H - Threshhold
#define peakthres1 2

// I - Influence
#define peakinfluence1 0.80

// F - Minimum jerk value to add
#define jerkthreshold1 1

// C - Sets the click sensitivity within the range
#define threshold1 30

// S - Shake threashold
#define shakeThreshold1 9.0

// J - Jerk threashold
#define accelThreshold1 600

// L - Jerk low threadshold
#define accelThresholdLow1 400

// T - Threshold for ignoring small accelerations (e.g., sensor resting)
#define restingThreshold1 0.2

// Sets the power mode, LIS3DH_MODE_NORMAL, LIS3DH_MODE_LOW_POWER, LIS3DH_MODE_HIGH_RESOLUTION
#define powermode1 LIS3DH_MODE_NORMAL

// Sets tap/click to 1 for single click or 2 for double click
#define clickPin1 1

// I2C pin designation for the Reflections board, change for your set-up
#define I2CSDA        3
#define I2CSCL        4

Adafruit_LIS3DH lis3dh = Adafruit_LIS3DH();

lis3dh_range_t range;
lis3dh_mode_t powermode;

// Acceleration readings
float accelerationX;
float accelerationY;
float accelerationZ;

float initialAccelX, initialAccelY, initialAccelZ;
float thresholdX, thresholdY, thresholdZ;

// Position readings
float rawX, accelX_g, accelX_ms2;
float rawY, accelY_g, accelY_ms2;
float rawZ, accelZ_g, accelZ_ms2;

bool tapdet;
bool shake;
bool peakTap;

unsigned long lastTapTime;
unsigned long debounceTapTime;
unsigned long debouncePeakTap;
unsigned long alabtimer;
unsigned long magtimer;
unsigned long cctime;
unsigned long lastTime;
unsigned long peakTimer;

float accelMagnitude;
float prevAccel;
int clickThreshold;
int clickPin;
float accelThreshold;    // high value for tap detection
float accelThresholdLow; // low value for tap detection
float shakeThreshold;    // for shake detection
float restingThreshold;  // Threshold for ignoring small accelerations (e.g., sensor resting)
bool restFlag;
float alpha;
int peak;
double filtered;

int rowCount;
long peakcnt;

PeakDetection peakDetection;
int peaklag;
int peakthres;
double peakinfluence;
double jerkthreshold;

void setup() 
{
  Serial.begin(115200);
  delay(1000);

  Serial.println( " " );
  Serial.println( " " );
  Serial.println( "Starting AccelerometerLab" );

  Wire.begin(I2CSDA, I2CSCL);  // Initialize I2C bus

  if ( !lis3dh.begin(0x18) )   // change this to 0x19 if alternative i2c address, see datasheet
  {
    Serial.println("LIS3DH did not start, stopping.");
    while (1);
  }

  Serial.println("LIS3DH sensor started");

  alabtimer = millis();
  magtimer = millis();
  shake = false;
  rowCount = 0;
  lastTapTime = millis();
  cctime = millis();
  prevAccel = 15000;
  debouncePeakTap = millis();
  peakTimer = millis();
  peakTap = false;
  tapdet = false;

  alpha = 0.1;            // Low-pass filter factor (adjustable)
  initialAccelX = 0;
  initialAccelY = 0;
  initialAccelZ = 0;
  lastTime = millis();
  restFlag = true;
  peakcnt = 0;

  range = range1;
  clickThreshold = threshold1;
  powermode = powermode1;
  clickPin = clickPin1;

  debounceTapTime = millis();

  accelThreshold = accelThreshold1;       // for tap detection
  accelThresholdLow = accelThresholdLow1; // for tap detection
  shakeThreshold = shakeThreshold1;       // for shake detection
  restingThreshold = restingThreshold1;   // Threshold for ignoring small accelerations (e.g., sensor resting)

  // Set shake detection parameters
  lis3dh.setRange( range );   // 2, 4, 8 or 16 G!

  // Low-speed movements (e.g., walking): 10–50 Hz.
  // Medium-speed movements (e.g., hand gestures): 50–100 Hz.
  // High-speed movements (e.g., vibrations, impacts): 200–400 Hz.

  lis3dh.setDataRate( datarate ); // Set ODR to 100 Hz

  // Set tap detection parameters
  lis3dh.setClick( clickPin1, clickThreshold );  // single click, threshold

  printSettings();

  delay(2000);

  printHeader();

  peaklag = peaklag1;
  peakthres = peakthres1;
  peakinfluence = peakinfluence1;
  jerkthreshold = jerkthreshold1;

/*
  peakDetection.begin(peaklag, peakthres, peakinfluence);

  double jerkValues[] = {193, 348, 9, 304, -212, 80, 239, 180, 81, 42, -597, 19, -201, 293, -27, -172, -102, 282, -130, 70};

  for (int i = 0; i < sizeof(jerkValues) / sizeof(jerkValues[0]); i++) 
  {
    double jerk = jerkValues[i];

    peakDetection.add( jerk );
    
    peak = peakDetection.getPeak();
    filtered = peakDetection.getFilt();

    Serial.print( i );
    Serial.print( ", " );
    Serial.print( jerk );
    Serial.print( ", " );
    Serial.print( filtered );
    Serial.print( ", " );
    Serial.println( peak );
      
    delay(500);
  }
  */

}

// Print settings to the Serial Monitor

void printSettings()
{
  Serial.println( " " );
  Serial.println( " " );
  Serial.println( "AccelerometerLab settings" );

  Serial.print( "Range " );
  Serial.println( range );
  Serial.print( "Shake threashold " );
  Serial.println( shakeThreshold );
  Serial.print( "Jerk threashold " );
  Serial.println( accelThreshold );
  Serial.print( "Jerk threashold low " );
  Serial.println( accelThresholdLow ); 

  lis3dh.setPerformanceMode( powermode );
  Serial.print("Accelerometer performance mode set to: ");
  switch (lis3dh.getPerformanceMode()) {
    case LIS3DH_MODE_NORMAL: Serial.println("Normal 10 bit"); break;
    case LIS3DH_MODE_LOW_POWER: Serial.println("Low Power 8 bit"); break;
    case LIS3DH_MODE_HIGH_RESOLUTION: Serial.println("High Resolution 12 bit"); break;
  }
}

// Function to print the header

void printHeader() 
{
  Serial.println("Enter R for range, A low-pass filter, C click/tap sensitivity, S shake threshold, J jerk threshold,");
  Serial.println( "X to reset the device, PeakDetection G lag, H threashold, I influence, F peak jerk threashold" );

  Serial.print("Example inputs: R ");

  if (range == LIS3DH_RANGE_2_G) 
  {
    Serial.print("2");
  } 
  if (range == LIS3DH_RANGE_4_G) 
  {
    Serial.print("4");
  } 
  if (range == LIS3DH_RANGE_8_G) 
  {
    Serial.print("8");
  } 
  if (range == LIS3DH_RANGE_16_G) 
  {
    Serial.print("16");
  }

  Serial.print( ", A ");
  Serial.print( alpha );

  Serial.print( ", C ");
  Serial.print( clickThreshold );

  Serial.print( ", S ");
  Serial.print( shakeThreshold );

  Serial.print( ", J ");
  Serial.print( accelThreshold );

  Serial.print( ", L ");
  Serial.print( accelThresholdLow );

  Serial.print( ", T ");
  Serial.print( restingThreshold );

  Serial.print( ", G ");
  Serial.print( peaklag );
  Serial.print( ", H ");
  Serial.print( peakthres );
  Serial.print( ", I ");
  Serial.print( peakinfluence );
  Serial.print( ", F ");
  Serial.println( jerkthreshold );

  Serial.println(" Acceleration                Raw                                                  PeakDetection");
  Serial.println(" X        Y       Z          X     Y      Z        Magni   Jerk  RestC Shake Tap  Filter Peak");
}

void resetLIS3DH() 
{
  // Read the current value of CTRL_REG5 (0x24)
  uint8_t ctrlReg5Value = 0;
  Wire.beginTransmission(0x18);  // Start communication with LIS3DH (I2C address)
  Wire.write(0x24);  // Register address for CTRL_REG5
  Wire.endTransmission(false);  // Keep the connection active
  Wire.requestFrom(0x18, 1);  // Request 1 byte of data
  if (Wire.available()) {
    ctrlReg5Value = Wire.read();  // Read the current value from CTRL_REG5
  }

  // Set the REBOOT_MEMORY bit (bit 7) to reset the device
  ctrlReg5Value |= 0x80;  // Set the most significant bit to 1 (0x80)

  // Write the new value back to CTRL_REG5 to reboot the sensor
  Wire.beginTransmission(0x18);  // Start communication again
  Wire.write(0x24);  // CTRL_REG5 register address
  Wire.write(ctrlReg5Value);  // Write the updated value
  Wire.endTransmission();  // End transmission

  // Wait for the reset to take effect
  delay(10);
}

/* Handles commands entered through the Serial Monitor */

void handleCommands()
{
  // Check if there's any data available in the Serial Monitor
  if (Serial.available() > 0) 
  {
    char input = Serial.read();  // Read the first character
    String value = "";           // To store the input value

    // Wait for the rest of the input (number)
    while (Serial.available() > 0) 
    {
      value += (char)Serial.read();  // Read the input value
      delay(10);  // Small delay to allow the Serial buffer to fill
    }

    if (input == 'X' || input == 'x') 
    {
      resetLIS3DH();
      Serial.println( "Reset accelerometer" );
      delay( 2000 );
    }

    // Handle the input based on the character received
    if (input == 'R' || input == 'r') 
    {
      // Set the range value
      int newRange = value.toInt();  // Convert the value to an integer
      
      if (newRange == 2 || newRange == 4 || newRange == 8 || newRange == 16) 
      {
        if (newRange == 2) 
        {
          lis3dh.setRange(LIS3DH_RANGE_2_G);
          range = LIS3DH_RANGE_2_G;
          Serial.println("Range set to: LIS3DH_RANGE_2_G");
          delay(2000);
        } 
        else if (newRange == 4) 
        {
          lis3dh.setRange(LIS3DH_RANGE_4_G);
          range = LIS3DH_RANGE_4_G;
          Serial.println("Range set to: LIS3DH_RANGE_4_G");
          delay(2000);
        } 
        else if (newRange == 8) 
        {
          lis3dh.setRange(LIS3DH_RANGE_8_G);
          range = LIS3DH_RANGE_8_G;
          Serial.println("Range set to: LIS3DH_RANGE_8_G");
          delay(2000);
        } 
        else if (newRange == 16) 
        {
          lis3dh.setRange(LIS3DH_RANGE_16_G);
          range = LIS3DH_RANGE_16_G;
          Serial.println("Range set to: LIS3DH_RANGE_16_G");
          delay(2000);
        }
      } 
      else 
      {
        Serial.println("Invalid input for Range. Please enter 2, 4, 8, or 16.");
        delay(2000);
      }
    }
    
    else if (input == 'S' || input == 's') 
    {
      // Set the shake threshold
      float newShakeThreshold = value.toFloat();  // Convert the value to a float
      if (newShakeThreshold > 0) 
      {
        shakeThreshold = newShakeThreshold;
        Serial.print("Shake threshold set to: ");
        Serial.println(shakeThreshold);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Shake threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'T' || input == 't') 
    {
      // Set the shake resting threshold
      float newResting = value.toFloat();  // Convert the value to a float
      if (newResting > 0) 
      {
        restingThreshold = newResting;
        Serial.print("Shake resting threshold set to: ");
        Serial.println( restingThreshold );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Resting Thhreshold filter. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'C' || input == 'c') 
    {
      // Set the click threshold
      float newClickThreshold = value.toFloat();  // Convert the value to a float
      if ( newClickThreshold > 0 ) 
      {
        clickThreshold = newClickThreshold;
        lis3dh.setClick( clickPin, clickThreshold);  // Set new click threshold
        Serial.print("Click threshold set to: ");
        Serial.println(clickThreshold);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Click threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'J' || input == 'j') 
    {
      // Set the acceleration threshold for shake detection
      float newAccelThreshold = value.toFloat();  // Convert the value to a float
      if (newAccelThreshold > 0) 
      {
        accelThreshold = newAccelThreshold;
        Serial.print("Acceleration threshold set to: ");
        Serial.println(accelThreshold);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Acceleration threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'L' || input == 'j') 
    {
      // Set the acceleration threshold for shake detection
      float newAccelThresholdLow = value.toFloat();  // Convert the value to a float
      if (newAccelThresholdLow > 0) 
      {
        accelThresholdLow = newAccelThresholdLow;
        Serial.print("Acceleration low threshold set to: ");
        Serial.println(newAccelThresholdLow);
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for Acceleration Low threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'G' || input == 'g') 
    {
      // Set the acceleration threshold for shake detection
      float newPeakLag = value.toFloat();  // Convert the value to a float
      if (newPeakLag > 0) 
      {
        peaklag = newPeakLag;
        peakDetection.begin(peaklag, peakthres, peakinfluence);
        Serial.print("PeakDetection lag threshold set to: ");
        Serial.println( newPeakLag );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for PeakDetection lag threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'H' || input == 'h') 
    {
      // Set the acceleration threshold for shake detection
      float newPT = value.toFloat();  // Convert the value to a float
      if (newPT > 0) 
      {
        peakthres = newPT;
        peakDetection.begin(peaklag, peakthres, peakinfluence);
        Serial.print("PeakDetection threshold set to: ");
        Serial.println( peakthres );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for PeakDetection lag threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'I' || input == 'i') 
    {
      // Set the acceleration threshold for shake detection
      float newPeakInflu = value.toFloat();  // Convert the value to a float
      if (newPeakInflu > 0) 
      {
        peakinfluence = newPeakInflu;
        peakDetection.begin(peaklag, peakthres, peakinfluence);
        Serial.print("PeakDetection peak influence set to: ");
        Serial.println( peakinfluence );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for PeakDetection lag threshold. Please enter a positive value.");
        delay(2000);
      }
    }

    else if (input == 'F' || input == 'f') 
    {
      // Set the jerk threshold peak recognition
      float newJT = value.toFloat();  // Convert the value to a float
      if (newJT > 0) 
      {
        jerkthreshold = newJT;
        Serial.print("Jerkthreshold set to: ");
        Serial.println( jerkthreshold );
        delay(2000);
      } 
      else 
      {
        Serial.println("Invalid input for jerkthreshold. Please enter a positive value.");
        delay(2000);
      }
    }
  }
}

void loop3()
{

}

void loop2()
{
  lis3dh.read();
  rawX = lis3dh.x;
  rawY = lis3dh.y;
  rawZ = lis3dh.z;
  accelMagnitude = sqrt( ( rawX * rawX ) + ( rawY * rawY ) + ( rawZ * rawZ ) );

  float jerk = accelMagnitude - prevAccel;
  prevAccel = accelMagnitude;

  double djerk = jerk;

  peakDetection.add( djerk );
  peakcnt++;

  // Show the results periodically

  peak = peakDetection.getPeak();
  filtered = peakDetection.getFilt();

  Serial.print( peakcnt );
  Serial.print( ", " );
  Serial.print( djerk );
  Serial.print( ", " );
  Serial.print( filtered );
  Serial.print( ", " );
  Serial.println( peak );
    
  delay(500);
}


void loop() 
{
  handleCommands();   // Process commands through the Serial Monitor

  // Check for tap/click detection

  if ( millis() - cctime > 2000 )
  {
    cctime = millis();

    uint8_t click = lis3dh.getClick();
    if ( click == 0 ) return;
    if ( ! ( click & 0x30 ) ) return;

    Serial.print( "Click detected (0x" );
    Serial.print( click, HEX ); 
    Serial.print( " " );

    for (int b = 7; b >= 0; b--)
    {
      Serial.print(bitRead(click, b));
    }  
    Serial.print("): ");

    if (click & 0x01) {
      Serial.print("X, ");
    }
    if (click & 0x02) {
      Serial.print("Y, ");
    }
    if (click & 0x04) {
      Serial.print("Z, ");
    }
    if (click & 0x08) {
      Serial.print("Sign, ");
    }
    if (click & 0x10) {
      Serial.print("Single click, ");
    }
    if (click & 0x20) {
      Serial.print("Double click ");
    }
    Serial.println(" ");
  }

  /* The getEvent() function gets the raw sensor data then convert
     into acceleration values in m/s². These are the values that represent
     how the sensor is moving in 3D space. */

  sensors_event_t event;
  lis3dh.getEvent(&event);

  accelerationX = event.acceleration.x;
  accelerationY = event.acceleration.y;
  accelerationZ = event.acceleration.z;
  
  if ( restFlag )
  {
    initialAccelX = accelerationX;
    initialAccelY = accelerationY;
    initialAccelZ = accelerationZ;
    restFlag = false;
  }

  if ( millis() - lastTime >= 2000 ) 
  {
    lastTime = millis();
    restFlag = true;

    // Calculate the difference between initial and final accelerations
    thresholdX = fabs(accelerationX - initialAccelX);
    thresholdY = fabs(accelerationY - initialAccelY);
    thresholdZ = fabs(accelerationZ - initialAccelZ);

    // Check if the accelerometer is at rest (small values close to zero)
    if ( ( fabs(accelerationX) < restingThreshold ) 
      && ( fabs(accelerationY) < restingThreshold )
      && ( fabs(accelerationZ) < restingThreshold ) ) 
    {
      // If the sensor is resting, ignore the shake detection
      Serial.println("Resting, no shake detected.");
    } 
    else 
    {
      // Check if the difference exceeds the shake threshold
      if ( ( thresholdX > shakeThreshold )
         || ( thresholdY > shakeThreshold )
         || ( thresholdZ > shakeThreshold ) )
      {
        shake = true;
      } 
      else 
      {
        shake = false;
      }
    }
  }

  lis3dh.read();
  rawX = lis3dh.x;
  rawY = lis3dh.y;
  rawZ = lis3dh.z;

  /* The read() method gives the raw acceleration values in ADC units or 
    counts (not in m/s²). These are unprocessed values from the sensor's internal
    registers. To convert them to physical units (like m/s² or g), the following
    applies a scaling factor based on the accelerometer's configuration (i.e., the set 
    range, such as ±2g, ±4g, etc.) */

  float scaleFactor = 16384.0; // For ±2g range
  if (range == LIS3DH_RANGE_2_G) scaleFactor = 16384.0;
  if (range == LIS3DH_RANGE_4_G) scaleFactor = 8192.0;
  if (range == LIS3DH_RANGE_8_G) scaleFactor = 4096.0;
  if (range == LIS3DH_RANGE_16_G) scaleFactor = 2048.0;

  accelX_g = rawX / scaleFactor;
  accelY_g = rawY / scaleFactor;
  accelZ_g = rawZ / scaleFactor;

  accelX_ms2 = accelX_g * 9.80665;
  accelY_ms2 = accelY_g * 9.80665;
  accelZ_ms2 = accelZ_g * 9.80665;

  // Calculate the magnitude of the acceleration in all directions
  accelMagnitude = sqrt( ( rawX * rawX ) + ( rawY * rawY ) + ( rawZ * rawZ ) );

  if ( millis() - magtimer < 10 ) return;
  magtimer = millis();

  // Calculate the jerk (rate of change of acceleration)
  float jerk = accelMagnitude - prevAccel;
  prevAccel = accelMagnitude;

  // Check for tap (sharp increase followed by quick return)

  if ( millis() - debounceTapTime > 1500 )
  {
    if ( millis() - lastTapTime > 50 )
    {
      lastTapTime = millis();

      if ( ( fabs( jerk ) <= accelThreshold ) && ( fabs( jerk ) > accelThresholdLow ) )
      {        
        tapdet = true;
        debounceTapTime = millis();
      }
    }
  }

  double djerk = jerk;

  if ( fabs( djerk ) > jerkthreshold )
  {
    peakDetection.add( djerk );
    peakcnt++;
  }

  // Show the results periodically

//  if ( millis() - alabtimer < 1000 ) return;
//  alabtimer = millis();

  peak = peakDetection.getPeak();
  filtered = peakDetection.getFilt();

    Serial.print( "value " );
    Serial.print( djerk );
    Serial.print( ", " );
    Serial.print( filtered );
    Serial.print( ", " );
    Serial.println( peak );


  // Every 10 rows, reprint the header
  rowCount++;
  if (rowCount >= 20) {
    rowCount = 0;  // Reset the counter
    //printHeader();  // Reprint the header
  }

  char output[200];  // Buffer to hold the formatted string

  /*
  sprintf(output, "%7.2f %7.2f %7.2f | %6.0f %6.0f %6.0f | %8.0f %6.0f %4.2f | %3d %3d | %4.0f  % d",
        accelerationX, accelerationY, accelerationZ,
        rawX, rawY, rawZ,
        accelMagnitude, jerk, restingThreshold,
        shake, tapdet,
        filtered, peakTap
        );
  */

  // Suitable for cvs import to a spreadsheet
  sprintf(output, "%7.2f,%7.2f,%7.2f,%6.0f,%6.0f,%6.0f,%8.0f,%6.0f,%4.2f,%3d,%3d,%4.0f,%d, %lu",
        accelerationX, accelerationY, accelerationZ,
        rawX, rawY, rawZ,
        accelMagnitude, jerk, restingThreshold,
        shake, tapdet,
        filtered, peak,
        peakcnt 
        );

  Serial.println(output);

  peakTap = false;
  tapdet = false;

  delay(500);
}