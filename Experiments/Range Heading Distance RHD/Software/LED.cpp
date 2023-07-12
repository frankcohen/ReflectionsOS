/*
 Reflections, distributed entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Depends on FastLED library
 Pins and LED type set in config.h

*/

#include "LED.h"

LED::LED(){}

void LED::begin()
{
  pinMode(LED_Pin, OUTPUT);

  FastLED.addLeds<LED_Type, LED_Pin>(leds, LED_Count);  // GRB ordering is assumed
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical


}

void LED::show()
{
  FastLED.show();
}

void LED::setBlue( int lednum )
{
  leds[ lednum ] = CRGB::Blue;
}

void LED::setAllBlack()
{
  for ( int i = 0; i< LED_Count; i++ )
  {
    leds[ i ] = CRGB::Black;
  }
}

void LED::loop()
{
}
