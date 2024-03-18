# Wrist Gesture Sensing
by Frank Cohen, fcohen@starlingwatch.com
March 17, 2024

Licensed under the Creative Commons, ok to share with attribution.

This is an experiment to see if a computing device can make sense of gestures made with a person's wrist. It idenfies the [Fabulous Four Gestures](#fabulous-four-gestures) of next, previous, accept, and cancel. It is extensible to train and identify additional gestures. It uses [ESP32](https://www.espressif.com/en/products/socs/esp32) and [LIS3DHTR](https://www.digikey.com/en/products/detail/stmicroelectronics/LIS3DHTR/2334338) accelerometer. It uses a [Movement Observation Matching](#movement-observation-matching) algorithm to make sense of a person's wrist movements.

## Fabulous Four Gestures

Human interfaces to computing devices often benefit from having 4 primary controls. I call these the Fabulous Four Gestures:

1. Next - person's goal is to move to a subsequent choice

Roll your fist counter clockwise, to the left.

![Next](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/Next.jpg)

2. Previous - person's goal is to return to a prior choice

Roll your fist clockwise, to the right.

![Previous](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/Previous.jpg)

3. Accept - person's goal is to approve a selection

I adopted the American Sign Language (ASL) movement to say yes, take a hand and make it into a fist and bob it back and forth, resembling a head nodding. 

![Accept](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/Accept.jpg)

4. Cancel - person's goal is to disapprove a selection

I adopted ASL to say no, take your first two fingers and tap them with your thumb, resembling a mouth saying no.

![Cancel](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/Cancel.jpg)



```
Make a gesture... 
Sensing gesture
Comparing gestures
Accellerometer gesture next     0.70, 0.74, 0.81, 0.83, 0.77,	 
Accellerometer gesture previous 0.67, 0.48, 0.39, 0.46, 0.55,	 
Accellerometer gesture cancel   0.55, 0.53, 0.57, 0.55, 0.58,	 
Accellerometer gesture accept   0.51, 0.49, 0.53, 0.57, 0.60,	 
Sum of averages, gesture recognized: next 
Highest average, gesture recognized: next 
```

In the case where no gesture is recognized the code shows:
```
Sum of averages, gesture inconclusive
Highest average, gesture inconclusive
```





## Movement Observation Matching

Movement Observation Matching (MOM) algorithm detects a person's wrist movements. It is a template system. It compares a person's movements to a library of recorded templates. Movement and template recordings begin with detection of a person's movement. A variety of techniques determine the conclusive determination of movements to the templates. 

Templates last 2 seconds, with 50 frames of accelerometer X, Y, Z data and 40 milliseconds between each frame. Recording begins after the person begins movement. Movements are the same duration and frames as templates.

Matching algorithms.

### Training


[@CieloStrive](https://github.com/CieloStrive) experiment on [gesture cognition using Dynamic Time Warping (DTW)](https://github.com/CieloStrive/GESTURE-RECOGNITION-DYNAMIC-TIME-WARPING) influenced this experiment. My DTW implementation is at [https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Gesture_Sensing_Accelerometer/AccelGestureDTW](https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Gesture_Sensing_Accelerometer/AccelGestureDTW)

#Code

Coded using 
Find the implementation in the ReflectionsOS repository here: [https://github.com/frankcohen/ReflectionsOS/blob/main/src/Accellerometer.cpp](https://github.com/frankcohen/ReflectionsOS/blob/main/src/Accellerometer.cpp)

# Building

Use Arduino IDE 2.3 or later. Code requires changes to be compatible with Arduino IDE 1.x nor Platform IO.

Requires these libraries:

LISDHTR Accelerometer, [https://travis-ci.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR.svg?branch=master](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR.svg?branch=master)

ESP32 board, [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)

Hardware wiring:

![Accelerometer schematic](Experiments/Gesture_Sensing_Accelerometer/LIS_schematic.jpg)

## Future 

We have these planned improvements to MOM:

- Template effectiveness learning. MOM recognizes and promotes templates correctly recognized, and denegrates and removes templates that do not recognize movements. It will do this by observing a person's movements after detecting a gesture. For example, when a person gestures "previous" after MOM detects a "next" gesture more than 50% of the time it is fair for MOM to identify the "next" gesture as incorrectly identified.

- Automatic new template detection. MOM recognizes new templates from movements that do not match templates. For example, when a person makes 5 "next" gestures then an unidentified new gestures then another 5 "next" gestures, MOM adds the unidentified movement gesture as a new "next" gesture.

- Template sharing between people. MOM shares templates with high success ratings among other devices.

## The Unkown

MOM has yet to be proven in these instances:

- Is there a difference in recognition between left and right handed people?

- What impact does a benign essential tremor (ET), Parkinson's disease, and Multiple Sclerosis (MS) have on gesture recognition?

- Do individuals gesture differently?

## ReflectionsOS Project

Reflections is an open-source hardware and software platform for building entertaining mobile experiences. Details at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS)

