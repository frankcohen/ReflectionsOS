/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

*/

#ifndef _config_
#define _config_

/* Experience videos */

#define OutOfTheBox_video "178e801340f79249dbb2b34b3e64eda4"     

#define Chastise_video "29eebc0d0804260f934575ceaedc50b1"

#define CatsPlayFound_video "5a9d9f72264522abc51300f9e772b136"
#define Pounce_video "17320773bbdcd9303f69d4dc4add38e6"        

#define CatsPlay1_video "001c0e74e770ec34c053879be10d1a7e"    // E
#define CatsPlay2_video "6216cf1923de998474b25afeda659448"    // NE
#define CatsPlay3_video "d5794454c11bca1f990e6a817e9b3b1e"    // N
#define CatsPlay4_video "6f119daa4433a28d44e19f37c4d9133f"    // NW
#define CatsPlay5_video "22ccc18e5b0c467fa209af54940b6773"    // W
#define CatsPlay6_video "98d5dacbc40ef6b1b8cfec4e5c6d230c"    // SW
#define CatsPlay7_video "bb0f5dbf32a9000787626cf71057fa26"    // S
#define CatsPlay8_video "6f19334918db04ddf84050696485c0d9"    // SE

#define EyesFollowFinger_video "f99c9b5031e6102ef45fbdf71cb09134"

#define MysticCat_video "fb6001b81b161fc556371a5bb34c17c9"

#define ParallaxCat_video "7f4f62c859d8a8638cb9126c23775372"

#define Sleep_video "72ec4d3eb423bd80ae8fd0c3bfd44143"

#define Shaken_video "89d7ee32f925de8a719726fbcacc622c"

#define Swipe_video "3d64a7500f82e9e3aa022318d17253ad"

#define EasterEggTerri_video "00e700137430e520ae75fb2b545eed31"

#define EasterEggFrank_video "a847ec268d727e816711caa1cb7e272c"

#define ShowTime_video "0c694842005c6e03d2abe845e530815e"

#define SetTime_video "c0fad0b76200936d73785cfc220d22cd"

#define Getting_Sleepy_video "69e2f5a6b7bfffab4a787be11f57fbcd"

#define videoname_end ".mjpeg"

// WatchFace_Main
#define wfMainBackground "Face_Main_background.jpg"
#define wfMainBattery "Battery_"   
#define wfMainBattery2 ".png"

#define wfMainHours "Hour"      
#define wfMainHours2 ".png"
#define wfMainMinutes "Minute"  
#define wfMainMinutes2 ".png"

#define wfMainFace "Blink_"    
#define wfMainFace2 ".png"
#define blinkspeed 100
#define flipspeed 50

#define wfMainHourglass "Main_HourGlass"    
#define wfMainHourglass2 ".png"

#define WatchFaceOpener_video "d1b0ec686a285bc1b75e268d62da0f49"    
#define WatchFaceFlip1_video "08ff4765f6c5cf764288afba9b4bcbbb"      
#define WatchFaceFlip2_video "358c45f36fba9802fdc26f67395119cc"      
#define WatchFaceFlip3_video "87af910e9dd542c183dc2e27a32d02e7"      

#define wfMain_Timer_Background "Face_Main_Timer_Background.jpg"     
#define wfMain_SetTimer_Background "Face_Main_SetTimer_Background.jpg"   
#define wfMain_SetTimer_Background_Shortie "Face_Main_SetTimer_Background_Shortie.jpg"   

#define wfMain_Health_Background "Face_Main_Health_Background.jpg"     

#define wfMain_SetTime_Background_Shortie "Face_Main_SetTime_Background_Shortie.jpg"     

#define wfMain_Time_Background "Face_Main_Blue_Dot_Background.jpg"
#define wfMain_Time_Background_Shortie "Face_Main_Time_Background_Shortie.jpg"   

#define wfMain_Face_Main_TimerNotice "Face_Main_TimerNotice.png"

#define wfMain_Face_BlueDot_Background "Face_Main_Blue_Dot_Background.jpg"     

#define wfMain_GPSmarker "GPSmarker.png"
#define wfMain_Wifimarker "WIFI.png"
#define wfMain_BLEmarker "BLE.png"

#define wfMain_BackgroundBlue RGB565( 0, 26, 112 )   // COLOR_PANTONE_662

#define TRANSPARENT_COLOR 0xFFE0
#define TRANSPARENT_COLOR_PNG 0x00000000

// Host identification
#define host_name_me "REFLECTIONS-"

// OTA uses this to know when to update the firmware
#define VERSION_NUMBER 2
#define OTA_VERSION_FILE_NAME "/otaversion.txt"
#define OTA_BIN_FILE_NAME "/ota.bin"

#define MJPEG_FILENAME ".mjpeg"
#define TAR_FILENAME ".tar"
#define JSON_FILENAME ".json"

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

// Accelerometer template storage
#define ACCEL_BASE_DIR "REFLECTIONS"
#define ACCEL_BASE_FILE "fantastic4"
#define ACCEL_BASE_EXT ".ages"

// Accellerometer
#define ACCEL_INT1_PIN GPIO_NUM_14
#define ACCEL_INT2_PIN GPIO_NUM_13
#define accelAddress 0x18     // 0x19 for alternative i2c address

// GPS
#define RXPin         18
#define TXPin         17
#define GPSPower      21

// Real Time Clock (RTC)
#define timeRegionOffset -1

// Gesture Time Of Flight (TOF)
#define TOFPower  26

// Audio
#define I2S_bclkPinP       9
#define I2S_wclkPinP      10
#define I2S_doutPinP       8
#define AudioPower         7

// I2C
#define I2CSDA        3
#define I2CSCL        4

// BLE

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define BLE_SERVER_UUID         "CATS"
#define BLE_CHARACTERISTIC_UUID "MEOW"
#define BLE_SERVER_NAME         "REFLECTIONS"

// LED

#define LED_Count 12
#define LED_Type NEOPIXEL
#define LED_Pin 39

// TOF Gesture Sensing

#define tof_detectionThreshold 1
#define tof_pollingInterval 1000
#define tof_majorityThreshold 1
#define tof_expiration 2000

// Battery

#define Battery_Sensor 16

/*
 The earth's magnetic field varies according to its location.
 Add or subtract a constant to get the right value
 of the magnetic field using the following site
 http://www.ngdc.noaa.gov/geomag-web/#declination
*/
#define DECLINATION -0.08387 // declination (in degrees) in Silicon Valley, California USA

// Cloud City REST services

// CloudCityURL defined in secrets.h

// Gets json format of available files
#define cloudCityListFiles "https://cloudcity.starlingwatch.com/api/listfiles"

// Recorder UX
#define cloudCityListRecorder "https://cloudcity.starlingwatch.com/"

// Show list of files in html view - with functions to download and delete
#define cloudCityFiles "https://cloudcity.starlingwatch.com/api/files"

// Logger URL
#define cloudCityLogURL "https://cloudcity.starlingwatch.com/api/logit?message="

// Logger POST URL
#define cloudCityLogPostURL "https://cloudcity.starlingwatch.com/api/logitpost"

/* New colors */

#define COLOR_PANTONE_310 RGB565( 91, 208, 230 )     // Pantone 310 C
#define COLOR_PANTONE_102 RGB565( 255, 236, 45 )       // Pantone 102 C
#define COLOR_PANTONE_577 RGB565( 20, 48, 15 )      // Pantone 577 C
#define COLOR_PANTONE_151 RGB565( 255, 130, 0 )       // Pantone 151 C
#define COLOR_PANTONE_282 RGB565( 4, 30, 66 )        // Pantone 282 C
#define COLOR_PANTONE_662 RGB565( 0, 26, 112 )     // Pantone 662 C
#define COLOR_PANTONE_313 RGB565( 0, 146, 188 )     // Pantone 313 C
#define COLOR_PANTONE_189 RGB565( 248, 163, 188 )     // Pantone 189 C
#define COLOR_PANTONE_205 RGB565( 224, 69, 123 )     // Pantone 205 C
#define COLOR_PANTONE_206 RGB565( 206, 0, 55 )     // Pantone 206 C
#define COLOR_PANTONE_265 RGB565( 206, 0, 55 )     // Pantone 206 C

/* Show Time text and colors */

#define COLOR_BACKGROUND RGB565(115, 58, 0)
#define COLOR_LEADING RGB565(123, 63, 0)
#define COLOR_RING RGB565(234, 68, 0)
#define COLOR_TRAILING RGB565(178, 67, 0 )
#define COLOR_TEXT RGB565( 234, 67, 0 )
#define COLOR_TEXT_BACKGROUND RGB565( 79, 42, 0)
#define COLOR_TEXT_BORDER RGB565( 207, 67, 0 )

#define COLOR_TEXT_YELLOW RGB565( 210, 200, 102 )
#define COLOR_STRIPE_MEDIUM_GRAY RGB565( 153, 105, 140 )
#define COLOR_STRIPE_PINK RGB565( 166, 139, 186 )
#define COLOR_BLUE RGB565( 0, 0, 140 )
#define COLOR_RED RGB565( 140, 0, 0 )
#define COLOR_BLACK RGB565(0, 0, 0)
#define COLOR_MAIN_BACK RGB565( 55, 161, 224 )

#define COLOR_EYES_LEFT RGB565( 192, 169, 28 )
#define COLOR_EYES_RIGHT RGB565( 207, 193, 36 )
#define COLOR_PUPILS RGB565( 0, 0, 0 )

#endif // _config_
