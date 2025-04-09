# AccelerometerLab for Gesture Sensing

### by Frank Cohen, fcohen@starlingwatch.com, January 15, 2025

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

Depends on these libraries:
- Adafruit LIS3DH library, https://github.com/adafruit/Adafruit_LIS3DH
- Adafruit Unified Sensor, https://github.com/adafruit/Adafruit_Sensor
- Adafrruit BusIO, I2C support, https://github.com/adafruit/Adafruit_BusIO
- PeakDetection, peak and valley detection, https://github.com/leandcesar/PeakDetection

Written in support of the Reflections project at:
https://github.com/frankcohen/ReflectionsOS

## Gesture Sensing

Let’s do a technical deep dive into the LIS3DH. Yeah, ok, wait… what?

### Why?

Good makers turn out creations that are scalable, sustainable, rapid, and ethical. That’s SERS. With SERS it’s not up to the human to learn the software, the software needs to understand the intentions of the human. When I move my arm up, you know intuitively what I’m trying to say. Something SERS needs to  understand the intention too.

LIS3DH is an inexpensive 3 Axis accelerometer sensor. It senses movement and acceleration from gravity. I use it to sense gestures in the Reflections open-source mobile entertainment platform for visual controls and to wake the ESP32-S3 processor from deep sleep. It senses movement, taps, clicks, shaking, and acceleration. Many libraries exist and generative AI such as ChatGPT is familiar and generates code for the sensor. 

While working with the sensor I didn’t find answers to a bunch of questions. For example, what are the best sensitivity and gravity values to use with the sensor? I wrote code to experiment with the sensor. I publish this library under an open-source license hoping you will contribute your LIS3DH experiences, tips, and techniques for others to enjoy.

In 10 seconds here’s how it works. The code starts up the sensor, displays the current gravity readings on the x, y, and z axis, shows the magnitude of movements, changes the sensitivity and other inputs through the Arduino IDE Serial Monitor dynamically, and shows 3 strategies to make sense of the readings.

![AccelerometerLab Screen Shot](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/AccelerometerLab/AccelLab_ScreenShot.jpg)

### Why 3?

The sensor spits out measurements and the code needs to turn these into understanding. Is the sensor being tapped, clicked, moved to the right or left, shaken? I haven’t found one approach that works for everything. Here’s what I found.

### PeakDetection

Uses a statistical method using standard deviation. I remember the look on my high school mathematics teacher Art when trying to get me to understand stats. This project is his revenge, I should have been paying more attention to you Art!

PeakDetection automatically adapts to the inputs. When I make slight movements at one time and vigorous ones later PeakDectection adapts. The downside is during its adaption it can spit out a bunch of false positive values.

### Simple Range Filtering

Uses a simple high and low level filter to detect expected movement. A lot fewer false positives, and not much flexibility. When I get onto a bouncy bus wearing the sensor the PeakDetection system would be better.

### Hardware Click and Movement Detection

Uses the click detection built into the sensor. LIS3DH detects single and double clicks. It detects general movement along x, y, and z axis’. Hardware detection is nice to have and not all that accurate. Chasing after different sensitivity level settings is not easy. This is the worst approach for false positives.

I like the LIS3DH for its versatility, including basic motion detection, like measuring the orientation of a device, to more advanced applications, like gesture recognition or even building activity tracking systems. It’s tiny in a surface mount 201 package that’s much smaller than the tip of your finger. It has very low power consumption and lets you balance between performance and power consumption. It’s great for battery powered projects. 

### What I Learned

Here are highlights from the experience using LIS3DH:

1) The sensor gives raw values based on gravity from X, Y, and Z axes making it versatile for high-sensitivity applications, like detecting small movements, and for applications that need to measure larger accelerations, like detecting fast motions.

2) Detecting simple gestures like shakes or taps is easy. Complex gestures like sensing when you move Magic 8 Ball up and down are possible. Monitor the accelerometer’s data, triggering an action when detecting a certain pattern of movement.

3) Handling noise is the biggest issue. I added various filtering techniques to smooth the data and remove irrelevant fluctuations. For example, a Kalman filter reduces noise and predicts movement of the accelerometer. We also had success using low-pass filters from data processing. The toughest detection is a single tap, and I wasn’t altogether successful weeding through all the noise.

4) Tracking activity, like when a person is resting, moving, or shaking was easy. Many fitness trackers do this. We were able to infer when someone was still and when they were moving, and even recognize shaking gestures.

I rail pretty hard on the Disney Parks application for not being scalable, sustainable, rapid, and ethical, for not being SERS. And yet there is this little LIS3DH sensor… like the little train that could. The sensor detects when I am standing in Tomorrowland, separate from Fantasyland, and helps software understand my intentions and needs. Imagine how this sensor helps to make my experience even more immersive. If only…

I'm glad to tell you what Clever Moe did for PepsiCo, Wynn Casinos, Best Buy, Rackspace, and many other organizations.

Check out the [Reflections project](https://github.com/frankcohen/ReflectionsOS) for building entertaining IOT based experiences.

Thanks!

