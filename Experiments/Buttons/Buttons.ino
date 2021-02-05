/*
 * Read the button status
 *
 * Board wiring directions and code at https://github.com/frankcohen/ReflectionsOS
 *
 * Reflections project: A wrist watch
 * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
 * player/recorder, SD card, GPS, and accelerometer/compass
 * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
 * Licensed under GPL v3
 * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
 * Read the license in the license.txt file that comes with this code.
 * February 5, 2021
 * 
 * Thanks to MicroControllersLab.com for the tutorial on digital inputs 
 * and pull-down resisters:
 * https://microcontrollerslab.com/push-button-esp32-gpio-digital-input/
*/

#define leftButton 36
#define centerButton 34
#define rightButton 39

void setup() {
  pinMode(36, INPUT);
  pinMode(39, INPUT);
  pinMode(34, INPUT);
  Serial.begin( 115200 );
}

void loop() {
  if ( digitalRead( 36 ) )
  {
    Serial.println ( "Left button pressed" );
  }
  
  if ( digitalRead( 34 ) )
  {
    Serial.println ( "Middle button pressed" );
  }
  
  if ( digitalRead( 39 ) )
  {
    Serial.println ( "Right button pressed" );
  }

  delay(1000);
}
