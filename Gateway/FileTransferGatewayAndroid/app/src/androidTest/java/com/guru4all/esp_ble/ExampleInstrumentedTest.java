/**
// * Transfer files between Seuss display board and Android mobile device using Bluetooth classic
// * This is the ESP32 side of the Gateway, it runs on the Seuss Display Board
// *
// * Board wiring directions and code at https://github.com/frankcohen/ReflectionsOS
// *
// * Reflections project: A wrist watch
// * Seuss Display: The watch display uses a breadboard with ESP32, OLED display, audio
// * player/recorder, SD card, GPS, and accelerometer/compass
// * Repository and community discussions at https://github.com/frankcohen/ReflectionsOS
// * Licensed under GPL v3
// * (c) Frank Cohen, All rights reserved. fcohen@votsh.com
// * Read the license in the license.txt file that comes with this code.

//   Thanks to Manoj Mishra for coding the Bluetooth file transfer protocol on Android and ESP32.
//   Contact Manoj at: Manoj Mishra, Indore, India, http://nevalosstechnologies.com/
*/

package com.guru4all.esp_ble;

import android.content.Context;

import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest {
    @Test
    public void useAppContext() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        assertEquals("com.guru4all.esp_ble", appContext.getPackageName());
    }
}
