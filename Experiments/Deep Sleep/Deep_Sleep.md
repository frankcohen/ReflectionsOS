# ESP32-S3 Deep Sleep

June 18, 2024 fcohen@starlingwatch.com

## Extending Battery Life for ESP32-S3 Projects

We are at a point in development of the [Reflections project](https://github.com/frankcohen/ReflectionsOS) where optimizations to get more battery time are important. Putting the ESP32 and display into deep sleep extends the battery life. This experiment shows the code needed and the pittfalls I encountered.

### Wake On Movement

Reflections board uses an [LIS3DH accelerometer](https://jlcpcb.com/partdetail/STMICROELECTRONICS-LIS3DHHTR/C2655077) to identify movement, taps, and clicks. It uses the ESP32-S3 Real Time Clock (RTC) interrupt pins to wake the processor from deep sleep. I use the [SparkFun LIS3DH](https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library) library and Arduino IDE 2.3.2.

I connect the ESP32-S3 and LIS32DH pins:

- GPIO12 to ACC INT1 pin 11
- GPIO13 to ACC INT2 pin 9

### Not all GPIOs are RTC pins, GPIO 14 is not

Originally I connected INT 1 from the accellerometer to GPIO 14. I assumed that all ESP interrupts from from mall GPIO pins. That's wrong according to the datasheet. To make the ESP interrupts work the GPIO needs to be one of the RTC compatible pins. GPIO 14 is not an RTC pin. Just my luck, as GPIO 0 to 21, except for 14, are RTC compatible pins.

Instead, I used GPIO 13 connected to INT 2. That works! However, using INT 2 requires extra configuration of the LIS3DH and that's not easy, as you will see from the code. I found the Adafruit library for LIS3DH doesn't easily support setting the custom parameters needed to use INT 2. I switched to using the Sparkfun library.

### Retaining Values in Deep Sleep

Deep sleep on ESP32 powers down everything (RAM, registers, etc.). I found this in the [datasheet](https://www.mouser.com/pdfdocs/esp-idf.pdf) "To retain the value of the wakecount variable across deep sleep cycles, you need to store it in a non-volatile memory area that persists through resets and power cycles. The ESP32 provides a feature called "RTC memory" that can be used for this purpose."

For example,

```
RTC_DATA_ATTR int wakecount;
```

creates wakecount in the RTC area of memory. Everything else gets reset during deep sleep.

### Setup and Loop and Waking-up

The first time a sketch runs it processes Setup() as expected from an Arduino sketch. Then it continually processes Loop().

```
void setup() 
{
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to open
  Serial.println("starting");
```
Once the ESP32-S3 wakes from a deep-sleep it has an option to process the Setup() differently.

```
void setup() 
{
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to open
  Serial.println("starting");

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if ( wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 ) 
  {
  	// Setup options from waking from deep sleep go here
  
  }
  else
  {
  	// Cold start setup options go here
	
  }
```

### LIS3DH INT 2 Configuration

Configuring LIS3DH for using INT2 takes a lot of code - this is crazy. Most of this comes from the Sparkfun LIS3DH library examples.

```
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
```

### ChatGPT and Deep Sleep

I used ChatGPT 4 to write the code in this experiment. I prompted it with:

```
Are you familiar with the ESP32S3 and the LIS3DHTR accelerometer? I have them connected together: I2C bus SDA on the accelerometer is connected to GPIO 3 and SCL connected to GPIO 4. The accelerometer INT1 pin is connected to GPIO 14 and INT2 pin is connected to GPIO 13. I need an Arduino IDE 2.3.2 sketch. It will setup the ESP32 and accelerometer communication, and then put the ESP32 into deep sleep. When the accelerometer experiences movement, it wakes the ESP32 from sleep and sends a notification to the Serial Monitor. After 15 seconds the ESP32S3 puts itself into deep sleep.
```

and it responded

```
Sure! Below is an Arduino IDE sketch for the ESP32S3 connected to the LIS3DHTR accelerometer as described. This sketch sets up the communication between the ESP32S3 and the LIS3DHTR, puts the ESP32S3 into deep sleep, and wakes it up when movement is detected by the accelerometer. After 15 seconds, the ESP32S3 goes back into deep sleep.
```

The code worked fine with LIS3DH using INT1. When I asked it for help using INT2 the experience went sideways.

```
It looks like I provided incorrect function signatures for the SparkFun LIS3DH library initialization. The correct initialization for the SparkFun LIS3DH library does not directly accept Wire and the address as arguments in that form.
```

My full exchange with ChatGPT is here:
[https://chatgpt.com/share/9b29f236-1812-4907-a1a0-d2cf79ffd39d](https://chatgpt.com/share/9b29f236-1812-4907-a1a0-d2cf79ffd39d)

I'm finding ChatGPT to be my coding partner everyday. It may not be perfect each time, it is much easier than reading datasheets and using Google search.

### Finished Code

Find these notes and a working sketch here: [https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Deep%20Sleep](https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Deep%20Sleep)

-Frank
