/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Hardware.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;

Hardware::Hardware(){}

void Hardware::begin()
{
  // Starts SD/NAND storage component

  pinMode( NAND_SPI_PWR, OUTPUT);
  digitalWrite( NAND_SPI_PWR, HIGH);
  delay(1000);
  digitalWrite( NAND_SPI_PWR, LOW);

  pinMode(NAND_SPI_CS, OUTPUT);
  digitalWrite(NAND_SPI_CS, HIGH);

  powerUpComponents(); 

  // Create an SPIClass instance
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, NAND_SPI_CS);

  if ( ! SD.begin( NAND_SPI_CS, SPI, SPI_SPEED) )
  {
    Serial.println(F("SD storage failed"));
    Serial.println(F("Stopping"));
    NANDMounted = false;
    video.stopOnError( F( "Storage" ), F( "failed" ), F( "to" ), F( "start" ), F( " " ) );
  }
  else
  {
    Serial.println(F("SD storage mounted"));
    NANDMounted = true;
  }

  // Begin SPI communication with desired settings
  //SPISettings spiSettings(SPI_SPEED, MSBFIRST, SPI_MODE0);

  Wire.begin(I2CSDA, I2CSCL);  // Initialize I2C bus
  Wire.setClock(400000);
}

/*

Hold GPIO21 (for GPS) during deep sleep:

What to use (and why)
Per-pin HOLD (…hold_en) “freezes” the pad’s current drive and config (level, direction, pulls).
Global deep-sleep HOLD keeps all held pads frozen across deep sleep entry/exit.
(Optional) Sleep-mode direction/level lets the RTC domain drive pins during sleep, but HOLD is the most bulletproof way to avoid any transient flips during the sleep transition.

Recommended sequence (no code yet)
Before entering deep sleep
Make sure GPIO21 is an OUTPUT and set it HIGH.
(You’re establishing the state you want to preserve.)
Enable per-pin HOLD for GPIO21.
This locks its current level (HIGH) and direction (OUTPUT).
Enable global deep-sleep HOLD.
This tells the chip to keep all held pads frozen during deep sleep.

Enter deep sleep.
The ON/OFF line will remain driven HIGH while the ESP32-S3 sleeps.

Right after waking up
Disable the global deep-sleep HOLD first, then disable per-pin HOLD on GPIO21.
Important: while a pad is held, writes/changes won’t take effect—so un-hold before reconfiguring the pin for normal operation (UART/GPIO/etc.).

Re-initialize GPIO21 as needed for run mode.
That’s it—the GPS ON/OFF pin will stay high the whole time.

Notes / gotchas
Persistence across resets: A held pad can stay latched even through a soft reset. If you ever see GPIO21 “stuck high” at boot, it’s because HOLD was still enabled—just disable it on startup before reconfiguring pins.
Power draw: Actively driving a line HIGH (vs. relying only on a pull-up) consumes a tiny bit more, but it guarantees a clean HIGH level to the GPS. If you want to minimize current further, you can add a hardware pull-up on the GPS ON/OFF pin so the ESP32 could safely switch to input+hold; but for positive, assertive control, output-HIGH+hold is preferred.
Why not only sleep-mode config? You can set “sleep direction = output” and “sleep level = high” (RTC-GPIO/gpio_sleep APIs), but a HOLD prevents glitches during the run→sleep transition and is simpler to reason about.
If you want, say “Show me the code” and I’ll drop in a compact Arduino-core snippet with the exact calls for ESP32-S3 GPIO21 (set high → hold → deep sleep → unhold on wake).
*/

void Hardware::prepareForSleep()
{
  // Freeze THIS pad’s current level/direction
  gpio_hold_en( GPIO_NUM_21 );    // GPSPower

  // Enable global deep-sleep hold so held pads stay frozen across sleep
  gpio_deep_sleep_hold_en();  
}

void Hardware::prepareAfterWake()
{
  // IMPORTANT: release global deep-sleep hold first
  gpio_deep_sleep_hold_dis();

  // Then release the per-pin hold
  gpio_hold_dis( GPIO_NUM_21 );   // GPSPower

  // Reapply your run-mode config

  pinMode(GPSPower, OUTPUT);     // Turn GPS module on
  digitalWrite(GPSPower, HIGH);  // HIGH is on
}

void Hardware::powerUpComponents()
{ 
  // Configure display pins

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);      // Turns backlight on = low, off = high
  digitalWrite(Display_SPI_BK, HIGH);

  pinMode( ACCEL_INT1_PIN, INPUT );     // Accelerometer awake on movement pins
  pinMode( ACCEL_INT2_PIN, INPUT );

  pinMode(LED_Pin, OUTPUT);

  pinMode( TOFPower, OUTPUT);       // Power control for TOF sensor
  digitalWrite( TOFPower, LOW);     // Low is on

  // Turns the speaker amp on
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);    // HIGH is on

  // Turn GPS module on
  Serial2.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  pinMode(GPSPower, OUTPUT);
  digitalWrite(GPSPower, HIGH);      // HIGH is on

  uint32_t seed = esp_random();
  randomSeed( seed );
}

void Hardware::powerDownComponents()
{
  pinMode(Display_SPI_BK, OUTPUT);  // Turns backlight on = low, off = high
  digitalWrite( Display_SPI_BK, HIGH );

  pinMode( TOFPower, OUTPUT);       // Power control for TOF sensor
  digitalWrite( TOFPower, HIGH);    // Low is on

  // Turns the speaker amp off
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);    // HIGH is on

  // Turn GPS module off
  //pinMode(GPSPower, OUTPUT);
  //digitalWrite(GPSPower, LOW);     // HIGH is on
}

bool Hardware::getMounted()
{
  return NANDMounted;
}

void Hardware::loop()
{
}
