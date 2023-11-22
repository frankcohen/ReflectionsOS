# ReflectionsOS Operations Notes

I use the NRDetector Android app to see BLE values coming from the main board

@dankeboy36 (on Github) published an Arduino stack trace decoding utility. When your sketch crashes a stack trace leading to the crash often appears in the Serial Monitor. Arduino IDE 2.1 does not yet support plug-ins and a decoder utility (available in Arduino IDE 1.8) is needed to explain the stack trace contents. @dankeboy36 created a decoder and it works great. It requires the nightly Arduino IDE build available at https://www.arduino.cc/en/software. The decoder is at at: https://github.com/dankeboy36/esp-exception-decoder. On a Mac use Command-Shift-P and type ESP Exception Decoder: Show Decoder Terminal.

## OTA updating added

Added Over The Air (OTA) update capabilitiy. Server downloads .bin firmware and a version identifier text file over HTTPS secure protocol. OTA system replaces the current firmware and restarts the host. Thanks to @lipun12ka4 on [Espressif Forums](https://esp32.com/posting.php?f=13&mode=reply&t=30973&sid=89252e0558c707c8813146f40cccf9fc) for sharing his OTA code, a problem, and solution. config.h defines the code's version number, it is a simple integer value. Arduino IDE compiles the firmware. Use FTP to store it in /home/fcohen/files on cloudcity.starlingwatch.com.

## Wifi connections added

Added [WifiManager](https://github.com/tzapu/WiFiManager) to enable easy connection to a Wifi network. Connect to Reflections board from your laptop or mobile device. The SSID is named CALLIOPE plus the last digits of the device mac address. For example, CALLIOPE-7A. Follow the instructions to log Reflections into your Wifi network. Passwords are sent to the device in the clear, and optionally stored on the device with no encryption. Someone could extract these if they got their hands on your device.

## MSC