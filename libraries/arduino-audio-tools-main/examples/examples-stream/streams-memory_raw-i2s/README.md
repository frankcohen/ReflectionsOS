# Digital output via I2S to a external DAC

Somtimes we want to store the sound file in memory. [Audacity](https://www.audacityteam.org/) might help you out here: export with the file name audio.raw as RAW signed 16 bit PCM and copy it to the SD card. In the example I was just using one channel to save memory!.

Then you can convert the file with xxd into a C file that contains the data in an array. In the Sketch I am using the __MemoryStream class__ which turns the array into a Stream. 

Unlike in the other examples I am using the typed __StreamCopyT<int16_t>__ class together with the __copy2()__ method. This reads the one channel input and copies it as 2 channels to the destination I2S stream.

Please note that you must compile this sketch with the __Partition Scheme: Huge App__!


### External DAC:

![DAC](https://pschatzmann.github.io/Resources/img/dac.jpeg)

For my tests I am using the 24-bit PCM5102 PCM5102A Stereo DAC Digital-to-analog Converter PLL Voice Module pHAT

I am just using the default pins defined by the framework. However I could change them with the help of the config object. The mute pin can be defined in the constructor of the I2SStream - by not defining anything we use the default which is GPIO23

 
DAC  |	ESP32
-----|----------------
VCC  |	5V
GND  |	GND
BCK  |	BCK (GPIO14)
DIN  |	OUT (GPIO22)
LCK  |	BCK (GPIO15)
FMT  |	GND
XMT  |	3V (or another GPIO PIN which is set to high)

- DMP - De-emphasis control for 44.1kHz sampling rate(1): Off (Low) / On (High)
- FLT - Filter select : Normal latency (Low) / Low latency (High)
- SCL - System clock input (probably SCL on your board).
- FMT - Audio format selection : I2S (Low) / Left justified (High)
- XMT - Soft mute control(1): Soft mute (Low) / soft un-mute (High)


