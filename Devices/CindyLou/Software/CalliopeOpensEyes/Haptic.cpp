/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on https://github.com/adafruit/Adafruit_DRV2605_Library

 Effects:
  1 − Strong Click - 100%
  2 − Strong Click - 60%
  3 − Strong Click - 30%
  4 − Sharp Click - 100%
  5 − Sharp Click - 60%
  6 − Sharp Click - 30%
  7 − Soft Bump - 100%
  8 − Soft Bump - 60%
  9 − Soft Bump - 30%
  10 − Double Click - 100%
  11 − Double Click - 60%
  12 − Triple Click - 100%
  13 − Soft Fuzz - 60%
  14 − Strong Buzz - 100%
  15 − 750 ms Alert 100%
  16 − 1000 ms Alert 100%
  17 − Strong Click 1 - 100%
  18 − Strong Click 2 - 80%
  19 − Strong Click 3 - 60%
  20 − Strong Click 4 - 30%
  21 − Medium Click 1 - 100%
  22 − Medium Click 2 - 80%
  23 − Medium Click 3 - 60%
  24 − Sharp Tick 1 - 100%
  25 − Sharp Tick 2 - 80%
  26 − Sharp Tick 3 – 60%
  27 − Short Double Click Strong 1 – 100%
  28 − Short Double Click Strong 2 – 80%
  29 − Short Double Click Strong 3 – 60%
  30 − Short Double Click Strong 4 – 30%
  31 − Short Double Click Medium 1 – 100%
  32 − Short Double Click Medium 2 – 80%
  33 − Short Double Click Medium 3 – 60%
  34 − Short Double Sharp Tick 1 – 100%
  35 − Short Double Sharp Tick 2 – 80%
  36 − Short Double Sharp Tick 3 – 60%
  37 − Long Double Sharp Click Strong 1 – 100%
  38 − Long Double Sharp Click Strong 2 – 80%
  39 − Long Double Sharp Click Strong 3 – 60%
  40 − Long Double Sharp Click Strong 4 – 30%
  41 − Long Double Sharp Click Medium 1 – 100%
  42 − Long Double Sharp Click Medium 2 – 80%
  43 − Long Double Sharp Click Medium 3 – 60%
  44 − Long Double Sharp Tick 1 – 100%
  45 − Long Double Sharp Tick 2 – 80%
  46 − Long Double Sharp Tick 3 – 60%
  47 − Buzz 1 – 100%
  48 − Buzz 2 – 80%
  49 − Buzz 3 – 60%
  50 − Buzz 4 – 40%
  51 − Buzz 5 – 20%
  52 − Pulsing Strong 1 – 100%
  53 − Pulsing Strong 2 – 60%
  54 − Pulsing Medium 1 – 100%
  55 − Pulsing Medium 2 – 60%
  56 − Pulsing Sharp 1 – 100%
  57 − Pulsing Sharp 2 – 60%
  58 − Transition Click 1 – 100%
  59 − Transition Click 2 – 80%
  60 − Transition Click 3 – 60%
  61 − Transition Click 4 – 40%
  62 − Transition Click 5 – 20%
  63 − Transition Click 6 – 10%
  64 − Transition Hum 1 – 100%
  65 − Transition Hum 2 – 80%
  66 − Transition Hum 3 – 60%
  67 − Transition Hum 4 – 40%
  68 − Transition Hum 5 – 20%
  69 − Transition Hum 6 – 10%
  70 − Transition Ramp Down Long Smooth 1 – 100 to 0%
  71 − Transition Ramp Down Long Smooth 2 – 100 to 0%
  72 − Transition Ramp Down Medium Smooth 1 – 100 to 0%
  73 − Transition Ramp Down Medium Smooth 2 – 100 to 0%
  74 − Transition Ramp Down Short Smooth 1 – 100 to 0%
  75 − Transition Ramp Down Short Smooth 2 – 100 to 0%
  76 − Transition Ramp Down Long Sharp 1 – 100 to 0%
  77 − Transition Ramp Down Long Sharp 2 – 100 to 0%
  78 − Transition Ramp Down Medium Sharp 1 – 100 to 0%
  79 − Transition Ramp Down Medium Sharp 2 – 100 to 0%
  80 − Transition Ramp Down Short Sharp 1 – 100 to 0%
  81 − Transition Ramp Down Short Sharp 2 – 100 to 0%
  82 − Transition Ramp Up Long Smooth 1 – 0 to 100%
  83 − Transition Ramp Up Long Smooth 2 – 0 to 100%
  84 − Transition Ramp Up Medium Smooth 1 – 0 to 100%
  85 − Transition Ramp Up Medium Smooth 2 – 0 to 100%
  86 − Transition Ramp Up Short Smooth 1 – 0 to 100%
  87 − Transition Ramp Up Short Smooth 2 – 0 to 100%
  88 − Transition Ramp Up Long Sharp 1 – 0 to 100%
  89 − Transition Ramp Up Long Sharp 2 – 0 to 100%
  90 − Transition Ramp Up Medium Sharp 1 – 0 to 100%
  91 − Transition Ramp Up Medium Sharp 2 – 0 to 100%
  92 − Transition Ramp Up Short Sharp 1 – 0 to 100%
  93 − Transition Ramp Up Short Sharp 2 – 0 to 100%
  94 − Transition Ramp Down Long Smooth 1 – 50 to 0%
  95 − Transition Ramp Down Long Smooth 2 – 50 to 0%
  96 − Transition Ramp Down Medium Smooth 1 – 50 to 0%
  97 − Transition Ramp Down Medium Smooth 2 – 50 to 0%
  98 − Transition Ramp Down Short Smooth 1 – 50 to 0%
  99 − Transition Ramp Down Short Smooth 2 – 50 to 0%
  100 − Transition Ramp Down Long Sharp 1 – 50 to 0%
  101 − Transition Ramp Down Long Sharp 2 – 50 to 0%
  102 − Transition Ramp Down Medium Sharp 1 – 50 to 0%
  103 − Transition Ramp Down Medium Sharp 2 – 50 to 0%
  104 − Transition Ramp Down Short Sharp 1 – 50 to 0%
  105 − Transition Ramp Down Short Sharp 2 – 50 to 0%
  106 − Transition Ramp Up Long Smooth 1 – 0 to 50%
  107 − Transition Ramp Up Long Smooth 2 – 0 to 50%
  108 − Transition Ramp Up Medium Smooth 1 – 0 to 50%
  109 − Transition Ramp Up Medium Smooth 2 – 0 to 50%
  110 − Transition Ramp Up Short Smooth 1 – 0 to 50%
  111 − Transition Ramp Up Short Smooth 2 – 0 to 50%
  112 − Transition Ramp Up Long Sharp 1 – 0 to 50%
  113 − Transition Ramp Up Long Sharp 2 – 0 to 50%
  114 − Transition Ramp Up Medium Sharp 1 – 0 to 50%
  115 − Transition Ramp Up Medium Sharp 2 – 0 to 50%
  116 − Transition Ramp Up Short Sharp 1 – 0 to 50%
  117 − Transition Ramp Up Short Sharp 2 – 0 to 50%
  118 − Long buzz for programmatic stopping – 100%
  119 − Smooth Hum 1 (No kick or brake pulse) – 50%
  120 − Smooth Hum 2 (No kick or brake pulse) – 40%
  121 − Smooth Hum 3 (No kick or brake pulse) – 30%
  122 − Smooth Hum 4 (No kick or brake pulse) – 20%
  123 − Smooth Hum 5 (No kick or brake pulse) – 10%

*/

#include "Haptic.h"

Haptic::Haptic(){}

void Haptic::begin()
{ 
  started = false;

  if( drv.begin() )
  {
    started = true;
    
    drv.selectLibrary(1);
    
    // I2C trigger by sending 'go' command 
    // default, internal trigger when sending GO command
    
    drv.setMode(DRV2605_MODE_INTTRIG);
  }
  else
  {
    Serial.println( F( "Haptic driver failed to start" ) );
  }
}

void Haptic::playEffect( int effect )
{
  // set the effect to play
  drv.setWaveform(0, effect);   // play effect 
  drv.setWaveform(1, 0);        // end waveform

  // play the effect
  drv.go();
}

bool Haptic::test()
{
  return started;
}

void Haptic::loop()
{
}
