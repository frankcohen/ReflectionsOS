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

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity2 extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main2);
        getSupportActionBar().hide();

        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Intent intent = new Intent(MainActivity2.this, MainActivity.class);
                startActivity(intent);
                finish();
            }
        },2000);
    }
}
