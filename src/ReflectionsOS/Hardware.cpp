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
#include "TOF.h"
#include "AccelSensor.h"

extern LOGGER logger;   // Defined in ReflectionsOS.ino
extern Video video;
extern Arduino_GFX *gfx;
extern TOF tof;
extern AccelSensor accel;

// --------------------------------------------------------------------------------------
// SLEEP POLICY
//
// Deep Sleep / Shipping Mode:
// - Entered by REST button, or by low battery at boot.
// - Lowest power usage possible.
// - Firmware wake sources are disabled.
// - Wake button is EN / CHIP_PU, so it wakes by hardware reset, outside ESP sleep wake logic.
// - USB / power reset also wakes.
//
// Sleep / Nap Mode:
// - Entered by TOF sleep gesture, inactivity timeout, or runtime low battery.
// - Lower power, but accelerometer INT1 remains available as an EXT1 wake source.
// - AccelSensor::configureWakeTapProfile() must be called before prepareForSleep().
//
// Board notes:
// - B1 / Wake pulls EN / CHIP_PU low. Firmware cannot read EN as a GPIO.
// - B2 / REST is IO0. Firmware can read IO0 and uses it to enter Shipping Mode.
// - Holding IO0 low while resetting can enter ROM download mode; release REST before Wake.
// --------------------------------------------------------------------------------------

static constexpr gpio_num_t kShipBtnGPIO = GPIO_NUM_0;  // IO0 button (B2 / REST)

// Optional but robust: freeze pin state through deep sleep to prevent glitches.
static void holdPin(gpio_num_t pin) { gpio_hold_en(pin); }
static void releasePin(gpio_num_t pin) { gpio_hold_dis(pin); }
static void enableDeepSleepHolds()  { gpio_deep_sleep_hold_en(); }
static void disableDeepSleepHolds() { gpio_deep_sleep_hold_dis(); }

static void releaseHeldPowerPins()
{
  disableDeepSleepHolds();

  releasePin((gpio_num_t)Display_SPI_BK);
  releasePin((gpio_num_t)Display_SPI_RST);
  releasePin((gpio_num_t)Display_SPI_CS);
  releasePin((gpio_num_t)Display_SPI_DC);

  releasePin((gpio_num_t)TOFPower);
  releasePin((gpio_num_t)AudioPower);
  releasePin((gpio_num_t)GPSPower);

  releasePin((gpio_num_t)NAND_SPI_PWR);
  releasePin((gpio_num_t)NAND_SPI_CS);
}

static void printShippingPinStates()
{
  Serial.printf(
    "Shipping pins: NAND_SPI_PWR=%d TOFPower=%d AudioPower=%d GPSPower=%d BK=%d RST=%d CS=%d DC=%d\n",
    digitalRead(NAND_SPI_PWR),
    digitalRead(TOFPower),
    digitalRead(AudioPower),
    digitalRead(GPSPower),
    digitalRead(Display_SPI_BK),
    digitalRead(Display_SPI_RST),
    digitalRead(Display_SPI_CS),
    digitalRead(Display_SPI_DC)
  );

  Serial.flush();
  delay(1000);
}

static void holdLowPowerPins()
{
  holdPin((gpio_num_t)Display_SPI_BK);
  holdPin((gpio_num_t)Display_SPI_RST);
  holdPin((gpio_num_t)Display_SPI_CS);
  holdPin((gpio_num_t)Display_SPI_DC);

  holdPin((gpio_num_t)TOFPower);
  holdPin((gpio_num_t)AudioPower);
  holdPin((gpio_num_t)GPSPower);

  printShippingPinStates();

  enableDeepSleepHolds();
}

// --------------------------------------------------------------------------------------

Hardware::Hardware(){}

void Hardware::begin()
{
  // If we woke by EN / Wake from Shipping Mode, deep-sleep GPIO holds may still be active.
  // Release them before trying to power storage/display/peripherals back up.
  releaseHeldPowerPins();

  // Starts SD/NAND storage component. Existing board behavior implies:
  //   HIGH = off / reset state
  //   LOW  = on
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

  // REST button (IO0). External pull-up exists (R27 10K), INPUT_PULLUP is fine too.
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
  // Release any retained sleep holds before applying run-mode pin states.
  releaseHeldPowerPins();

  pinMode(0, INPUT_PULLUP);     // Enable REST / IO0 button

  // NAND / SD power ON. Existing begin() pattern implies LOW is ON.
  pinMode(NAND_SPI_PWR, OUTPUT);
  digitalWrite(NAND_SPI_PWR, LOW);    // LOW = NAND/SD available on this board

  pinMode(NAND_SPI_CS, OUTPUT);
  digitalWrite(NAND_SPI_CS, HIGH);

  // Configure display pins
  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);      // Turns backlight on = low, off = high
  digitalWrite(Display_SPI_BK, HIGH);   // Default OFF at startup; display code turns it on when needed

  pinMode( ACCEL_INT1_PIN, INPUT );     // Accelerometer wake/movement pins
  pinMode( ACCEL_INT2_PIN, INPUT );

  pinMode(LED_Pin, OUTPUT);
  digitalWrite(LED_Pin, LOW);

  pinMode( TOFPower, OUTPUT);           // Power control for TOF sensor
  digitalWrite( TOFPower, LOW );         // LOW = TOF on

  // Speaker amp power
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, HIGH);        // Your comment says HIGH is on -> LOW is OFF

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

  // Put display controller into reset to reduce display-side current.
  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, LOW);

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, LOW);

  // TOF OFF. HIGH = on, so LOW = off.
  pinMode(TOFPower, OUTPUT);
  digitalWrite(TOFPower, HIGH);

  // Speaker amp OFF (HIGH is on -> LOW is off)
  pinMode(AudioPower, OUTPUT);
  digitalWrite(AudioPower, LOW);

  // NAND / SD lowest-current state from bench testing.
  // NOTE: LOW is also the run state, but it tested far lower-drain than HIGH in Shipping Mode.
  // Do not hold this pin in deep sleep until the power circuit polarity/back-powering is fully verified.
  pinMode(NAND_SPI_PWR, OUTPUT);
  digitalWrite(NAND_SPI_PWR, HIGH);

  pinMode(NAND_SPI_CS, OUTPUT);
  digitalWrite(NAND_SPI_CS, HIGH);

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

// Lowest-power deep sleep. Used by Shipping Mode and boot-time low battery.
// This intentionally disables firmware wake sources. Wake is EN / USB / power reset.
static void startLowestPowerDeepSleep()
{
  printShippingPinStates();

  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  Serial.println(F("Entering ESP deep sleep now"));
  Serial.flush();
  delay(500);

  esp_deep_sleep_start();

  while (true) { delay(1000); }
}

// ----------------------------------------
// Shipping mode entry: REST -> message -> lowest-power deep sleep
// ----------------------------------------
void Hardware::enterShippingMode()
{
  Serial.println( F( "Shipping mode started" ) );

  video.displayTextMessage( F( "I disappear" ), F( "only when you're" ), F( "exactly where" ), F("you need to be.") );
  delay( 1000 * 13 );

  Serial.println(F("Stopping sensors for shipping mode"));
  tof.setStatus(false);
  accel.setStatus(false);
  delay(100);

  // Hard power-down everything that can heat or drain.
  powerDownComponents();

  startLowestPowerDeepSleep();
}

// ----------------------------------------
// Boot-time low battery: no vanishing message, just protect the battery.
// ----------------------------------------
void Hardware::enterLowBatteryShippingMode()
{
  Serial.println( F( "Low battery shipping/deep sleep started" ) );

  powerDownComponents();

  startLowestPowerDeepSleep();
}

// ----------------------------------------
// Sleep helpers used by normal Sleep / Nap Mode.
// AccelSensor::configureWakeTapProfile() must be called before this method.
// Do NOT disable wake sources here; accelerometer EXT1 wake must remain armed.
// ----------------------------------------
void Hardware::prepareForSleep()
{
  powerDownComponents();

  // Lock off-states across deep sleep. Accelerometer remains available because
  // it is not powered through one of these controlled rails.
  holdLowPowerPins();
}

void Hardware::prepareAfterWake()
{
  rtc_gpio_pulldown_dis((gpio_num_t)ACCEL_INT1_PIN);
  rtc_gpio_pullup_dis((gpio_num_t)ACCEL_INT1_PIN);

  releaseHeldPowerPins();

  // Reapply run-mode config (GPS still OFF always)
  powerUpComponents();
}

// ----------------------------------------
// REST button detector on IO0 (B2)
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

    // Wait indefinitely until REST is released. This avoids IO0 being held low
    // during the EN/Wake reset later, which could enter bootloader mode.
    while (digitalRead(0) == LOW)
    {
      delay(1);   // yield to avoid watchdog issues
    }

    Serial.println(F("Rest released: entering Vanishing Sleep"));

    enterShippingMode();   // never returns
  }
}
