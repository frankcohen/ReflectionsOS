/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#include "Hardware.h"

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include "driver/rtc_io.h"

extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino
extern Video video;
extern Arduino_GFX *gfx;

// --------------------------------------------------------------------------------------
// SHIPPING MODE (IO0 long-press -> progress UI -> deep sleep with *no* wake sources)
//
// Your schematic:
// - B1/BOOT is EN (chip enable/reset). Firmware cannot measure long-press on EN.
// - B2/RESET is IO0. Firmware CAN measure long-press on IO0.
//
// Bootloader gotcha:
// - If IO0 is held LOW while you press EN, ESP32 may boot into ROM download mode.
//   So to wake: RELEASE B2 (IO0), then press B1 (EN).
// --------------------------------------------------------------------------------------

static constexpr gpio_num_t kShipBtnGPIO = GPIO_NUM_0;  // IO0 button (B2)
static constexpr uint32_t   kShipHoldMs  = 2000;        // 2 seconds
static constexpr uint32_t   kShipDebounceMs = 250;      // show UI only after deliberate hold

static String buildDots(uint8_t filled, uint8_t total)
{
  String s;
  s.reserve(total * 3);
  for (uint8_t i = 0; i < total; i++)
  {
    s += (i < filled) ? "●" : "○";
  }
  return s;
}

// Optional but robust: freeze pin state through deep sleep to prevent glitches
static void holdPin(gpio_num_t pin) { gpio_hold_en(pin); }
static void enableDeepSleepHolds()  { gpio_deep_sleep_hold_en(); }
static void disableDeepSleepHolds() { gpio_deep_sleep_hold_dis(); }

// --------------------------------------------------------------------------------------

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

  Wire.begin(I2CSDA, I2CSCL);  // Initialize I2C bus
  Wire.setClock(400000);

  // Shipping button (IO0). External pull-up exists (R27 10K), INPUT_PULLUP is fine too.
  pinMode((int)kShipBtnGPIO, INPUT_PULLUP);
}

// -------------------------
// GPS policy: OFF always
// -------------------------
void Hardware::setGPSAlwaysOff()
{
  pinMode(GPSPower, OUTPUT);

  // Your comment says: HIGH is on.
  // So OFF = LOW.
  digitalWrite(GPSPower, LOW);
}

// ----------------------------------------
// Components power control
// ----------------------------------------
void Hardware::powerUpComponents()
{
  pinMode(0, INPUT_PULLUP);     // Enable BOOT button

  // Configure display pins

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);      // Turns backlight on = low, off = high
  digitalWrite(Display_SPI_BK, HIGH);   // Default OFF at startup (you can turn on when needed)

  pinMode( ACCEL_INT1_PIN, INPUT );     // Accelerometer awake on movement pins
  pinMode( ACCEL_INT2_PIN, INPUT );

  pinMode(LED_Pin, OUTPUT);
  digitalWrite(LED_Pin, LOW);

  pinMode( TOFPower, OUTPUT);           // Power control for TOF sensor
  digitalWrite( TOFPower, LOW );        // Your existing code implied LOW is on

  // Speaker amp power
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);        // Your comment says HIGH is on -> LOW is OFF

  // GPS forced OFF always
  setGPSAlwaysOff();

  uint32_t seed = esp_random();
  randomSeed( seed );
}

void Hardware::powerDownComponents()
{
  // Backlight OFF (backlight on = LOW, off = HIGH)
  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, HIGH);

  // TOF OFF (you commented "Low is on", so High is off)
  pinMode(TOFPower, OUTPUT);
  digitalWrite(TOFPower, HIGH);

  // Speaker amp OFF (HIGH is on -> LOW is off)
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);

  // GPS OFF always
  setGPSAlwaysOff();

  // LED off
  pinMode(LED_Pin, OUTPUT);
  digitalWrite(LED_Pin, LOW);
}

bool Hardware::getMounted()
{
  return NANDMounted;
}

// ----------------------------------------
// Shipping mode entry (uninterruptible)
// ----------------------------------------
void Hardware::enterShippingMode()
{
  Serial.println( F( "Shipping mode started" ) );

  video.displayTextMessage( F( "I disappear" ), F( "only when you're" ), F( "exactly where" ), F("you need to be.") );
  delay( 1000 * 13 );

  // Hard power-down everything that can heat
  powerDownComponents();

  // Hold OFF states to prevent any run->sleep transition glitches
  holdPin((gpio_num_t)Display_SPI_BK);
  holdPin((gpio_num_t)TOFPower);
  holdPin((gpio_num_t)AudioPower);
  holdPin((gpio_num_t)GPSPower);
  enableDeepSleepHolds();

  // Disable *all* wake sources; only EN reset / power-cycle can wake now
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  Serial.flush();
  esp_deep_sleep_start();

  // never returns
}

// ----------------------------------------
// Sleep helpers used elsewhere (kept safe)
// ----------------------------------------
void Hardware::prepareForSleep()
{
  // Keep consistent with "GPS OFF always"
  powerDownComponents();

  // Lock off-states across deep sleep
  holdPin((gpio_num_t)Display_SPI_BK);
  holdPin((gpio_num_t)TOFPower);
  holdPin((gpio_num_t)AudioPower);
  holdPin((gpio_num_t)GPSPower);
  enableDeepSleepHolds();
}

void Hardware::prepareAfterWake()
{
  rtc_gpio_pulldown_dis((gpio_num_t)ACCEL_INT1_PIN);
  rtc_gpio_pullup_dis((gpio_num_t)ACCEL_INT1_PIN);

  // Release global hold first
  disableDeepSleepHolds();

  // Then release per-pin holds
  gpio_hold_dis((gpio_num_t)Display_SPI_BK);
  gpio_hold_dis((gpio_num_t)TOFPower);
  gpio_hold_dis((gpio_num_t)AudioPower);
  gpio_hold_dis((gpio_num_t)GPSPower);

  // Reapply run-mode config (GPS still OFF always)
  powerUpComponents();
}

// ----------------------------------------
// Long-press detector on IO0 (B2) w/ progress indicator
// ----------------------------------------
void Hardware::loop()
{
  static bool committed = false;

  if (committed) return;

  // REST button (IO0) is active-low
  if (digitalRead(0) == LOW)
  {
    committed = true;

    Serial.println(F("Rest pressed: waiting for release"));

    // Wait indefinitely until REST is released
    while (digitalRead(0) == LOW)
    {
      delay(1);   // yield to avoid watchdog issues
    }

    Serial.println(F("Rest released: entering Vanishing Sleep"));

    enterShippingMode();   // never returns
  }
}