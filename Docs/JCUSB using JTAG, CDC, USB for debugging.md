# ESP32 USB is frustrating, try JCUSB for S3 + USB + CDC + OpenOCD + Arduino IDE

YouTube creator and Arduino Expert Andreas Spiess recently published a video on using the USB debugging support on ESP32 for Arduino users at https://www.youtube.com/watch?v=hJSBTFsOnoA&t=0s. He presents 4 different ways to use USB with ESP32. His video culminates the struggle many have had with ESP32 support over the past 7 years since Espressif launched ESP32. I started coding in the 1970s and waited for an Arduino that has a great IDE, networking (Wifi and Bluetooth), lots of memory, debugging, and a community making useful libraries. The latest USB support in ESP32-S3 finally meets my needs.

However, Andreas' video on USB is like my Arduino experience overall. I found tremendous good will and very little coordination. For someone like me there still is no "Start Here" in Arduino. It's a mash-up of C programming (ugh, C should have been retired decades ago) and hardware variations. Andreas presents 4 ways ESP32 supports USB, and 1 of them meets my needs.

##JCUSB

I call this 1 solution JCUSB short for JTAG + CDC + USB + OpenOCD + Arduino IDE Debugger. Only ESP32-S3 and its successors support JCUSB - it won't work on ESP32 and ESP32-S2. JCUSB uses a custom USB cable to connect an ESP32 to a Windows or MacOS computer. Here is the custom cable:

![JCUSB Cable](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/UCuSB_cable.jpg)

Install Arduino IDE 2.1 or greater, choose the ESP32S3 Dev Kit board, then choose these settings in the Tools drop-down menu:

![JCUSB Arduino IDE 2.1 settings for ESP32-S3 board](https://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/ESP32-S3_Board_Settings.jpg)

Choosing a different board may cause Arduino IDE to not show you the needeed JTAG, CDC, and OpenOCD parameters needed for JCUSB to function.

Choose the ESP32 S3 Board type and serial port. Click compile, then click Debug.

![Arduinno IDE 2.1 Debugger](xhttps://github.com/frankcohen/ReflectionsOS/blob/main/Docs/images/Arduino_21_Debugger.jpg)

In Arduino IDE 2.1's Debug panel set debug breakpoints, inspeect variables and step through execution of your code.

JCUSB gives fast uploads, debugging, breakpoints, variable inspection, and stepping in and out of code. The alternative is to load up my code with  Serial.println(""); commands and compile-upload-run.

I created a video answer to Andreas with more discussion of the problems I encountered: https://www.youtube.com/watch?v=vQBxMgNvwZI

I am using JCUSB in my upcoming open source project Reflections, a portable and mobile way to make animation experiences possible. Check https://github.com/frankcohen/ReflectionsOS for source code, examples, experiments, and details.

And please let me know where I got any of this wrong, and how to fix it. 

Thanks!

-Frank
