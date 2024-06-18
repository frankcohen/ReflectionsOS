#include <Wire.h>
#include <SparkFunLIS3DH.h>
#include <esp_sleep.h>
#include <Arduino_GFX_Library.h>
#include "SD.h"
#include "SPI.h"

#include "FreeSerif8pt7b.h"
#include "FreeSansBold10pt7b.h"
#include "MKXTitle20pt7b.h"
#include "SomeTimeLater20pt7b.h"
#include "Minya16pt7b.h"
#include "ScienceFair14pt7b.h"

// Display
#define Display_SPI_DC    5
#define Display_SPI_CS    12
#define Display_SPI_RST   0
#define Display_SPI_BK    6

// SPI Bus
#define SPI_MOSI      35
#define SPI_MISO      37
#define SPI_SCK       36
#define SPI_SPEED     10000000

// NAND Storage
#define NAND_SPI_CS   15
#define NAND_SPI_PWR  11
#define NAND_BASE_DIR "REFLECTIONS"
#define LOGNAME_START "/REFLECTIONS/log"
#define LOGNAME_END ".txt"

// I2C
#define I2CSDA        3
#define I2CSCL        4

// Interrupt pins
#define INT1_PIN GPIO_NUM_14
#define INT2_PIN GPIO_NUM_13

#define LIS3DH_INT2_CFG               0x34
#define LIS3DH_INT2_SRC               0x35
#define LIS3DH_INT2_THS               0x36
#define LIS3DH_INT2_DURATION          0x37

Arduino_DataBus *bus = new Arduino_HWSPI(Display_SPI_DC, Display_SPI_CS, SPI_SCK, SPI_MOSI, SPI_MISO);
Arduino_GC9A01 *gfx = new Arduino_GC9A01( bus, Display_SPI_RST, 1 /* rotation */, false /* IPS */ );

/*
When using the deep sleep capability of the ESP32-S3, the microcontroller effectively powers down, 
losing any data stored in its RAM, including the values of variables. To retain the value of the 
wakecount variable across deep sleep cycles, you need to store it in a non-volatile memory area that 
persists through resets and power cycles. The ESP32 provides a feature called "RTC memory" that 
can be used for this purpose.
*/

RTC_DATA_ATTR int wakecount;

#define COLOR_BACKGROUND RGB565(115, 58, 0)
#define COLOR_LEADING RGB565(123, 63, 0)
#define COLOR_RING RGB565(234, 68, 0)
#define COLOR_TRAILING RGB565(178, 67, 0 )
#define COLOR_TEXT RGB565( 234, 67, 0 )
#define COLOR_TEXT_BACKGROUND RGB565( 79, 42, 0)
#define COLOR_TEXT_BORDER RGB565( 207, 67, 0 )

// Create an instance of the accelerometer
LIS3DH myIMU(I2C_MODE, 0x18); //Default constructor is I2C, addr 0x19.

void setup() 
{
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to open
  Serial.println("starting");

  pinMode(INT2_PIN, INPUT);

  pinMode( NAND_SPI_PWR, OUTPUT);
  digitalWrite( NAND_SPI_PWR, HIGH);
  delay(2000);
  digitalWrite( NAND_SPI_PWR, LOW);

  pinMode(NAND_SPI_CS, OUTPUT);
  digitalWrite(NAND_SPI_CS, HIGH);

  pinMode(Display_SPI_CS, OUTPUT);
  digitalWrite(Display_SPI_CS, HIGH);

  pinMode(Display_SPI_DC, OUTPUT);
  digitalWrite(Display_SPI_DC, HIGH);

  pinMode(Display_SPI_RST, OUTPUT);
  digitalWrite(Display_SPI_RST, HIGH);

  pinMode(Display_SPI_BK, OUTPUT);
  digitalWrite(Display_SPI_BK, LOW);

  // Initialize the SPI bus with custom pins
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, NAND_SPI_CS);
  
  Wire.begin(I2CSDA, I2CSCL);  // Initialize I2C bus
  
  if (myIMU.begin() != 0) {
    Serial.println("Could not start LIS3DH");
    while (1);
  }

  myIMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  myIMU.settings.accelRange = 2;      //Max G force readable.  Can be: 2, 4, 8, 16

  myIMU.settings.adcEnabled = 0;
  myIMU.settings.tempEnabled = 0;
  myIMU.settings.xAccelEnabled = 1;
  myIMU.settings.yAccelEnabled = 1;
  myIMU.settings.zAccelEnabled = 1;

  myIMU.applySettings();

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
  //dataToWrite |= 0x80; //Click detect on pin 1
  dataToWrite |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
  dataToWrite |= 0x20; //AOI2 event ()
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
  esp_sleep_enable_ext1_wakeup(BIT(INT2_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if ( wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 ) 
  {
    Serial.println("ESP32 woke up from deep sleep due to an external wake-up (movement detected).");

    /*
    gfx->begin();
    gfx->invertDisplay( true );
    gfx->fillScreen( COLOR_BACKGROUND );
    printCentered( 100, "Setup", YELLOW, &Some_Time_Later20pt7b );
    */

    return;
  } 
  else 
  {
    // This is a cold start
    Serial.println("ESP32 cold start...");
    wakecount = 0;

    gfx->begin();
    gfx->invertDisplay( true );
    gfx->fillScreen(BLUE);
    printCentered( 100, "Cold", YELLOW, &Some_Time_Later20pt7b );

    delay(5000);
//    Serial.println("ESP32 entering deep sleep...");
//    esp_deep_sleep_start();
  }
}

void printCentered( int y2, String text, uint16_t color, const GFXfont * font )
{
  gfx->setFont( font );

  int16_t x1, y1;
  uint16_t w, h;

  gfx->getTextBounds( text, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (gfx->width() - w) / 2;
  //int16_t y = (gfx->height() - h) / 2;
  //y += y2;

  gfx->setCursor(x, y2);
  gfx->setTextColor( color );
  //gfx->drawRect(x1 - 1, y1 - 1, w + 2, h + 2, YELLOW);
  gfx->println( text );
}

void loop() {
  // This will only run when the ESP32 wakes up

  wakecount++;

  gfx->begin();
  gfx->invertDisplay( true );
  gfx->fillScreen( COLOR_BACKGROUND );

  printCentered( 100, "Good morning!", YELLOW, &Some_Time_Later20pt7b );

  printCentered( 140, String( wakecount ), RED, &Minya16pt7b );

  // Wait for 5 seconds
  delay(5000);

  printCentered( 180, "Good night", GREEN, &ScienceFair14pt7b );

  delay( 2000 );

  esp_deep_sleep_start();

  delay( 2000 );
}
