# Wrist Gesture Sensing
by Frank Cohen, fcohen@starlingwatch.com
March 17, 2024, updated March 27, 2024

Licensed under GPL v3, ok to share with attribution.

This is an experiment to see if a computing device can make sense of gestures made with a person's wrist. It identifies the [Fabulous Four Gestures](#fabulous-four-gestures) of next, previous, accept, and cancel. It is extensible to train and identify additional gestures. It uses [ESP32](https://www.espressif.com/en/products/socs/esp32) and [LIS3DHTR](https://www.digikey.com/en/products/detail/stmicroelectronics/LIS3DHTR/2334338) accelerometer. It uses a [Movement Observation Matching](#movement-observation-matching) algorithm to make sense of a person's wrist movements.

## Wrist Gestures Briefing

5 minute video briefing on Wrist Gesture Sensing.

[![Briefing](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/Gesture_briefing_title_card.jpg)](https://youtu.be/nNwV_FyjRPc)

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

Serial monitor shows status and progress to recognize gestures.

```
Comparing gestures
DTWgpt accelerometer gesture next	    5090.33,	4006.13,	3806.27,	3491.63,	3926.08,	 
DTWgpt accelerometer gesture previous	3884.40,	3595.16,	3730.78,	3616.00,	3515.38,	 
DTWgpt accelerometer gesture cancel	    5265.93,	4413.33,	5127.32,	5331.95,	5711.88,	 
DTWgpt accelerometer gesture accept	    4579.27,	5161.31,	4865.67,	5162.56,	4810.36,	 
DTWgpt, gesture recognized: next 3491
```

In the case where no gesture is recognized the code shows:
```
DTWgpt, gesture recognized: Inconclusive
```

## Movement Observation Matching

Movement Observation Matching (MOM) algorithm detects a person's wrist movements. It is a template system. It compares a person's movements to a library of recorded templates. Movement and template recordings begin with detection of a person's movement. A variety of techniques determine the conclusive determination of movements to the templates. 

Templates last 2 seconds, with 50 frames of accelerometer X, Y, Z data and 40 milliseconds between each frame. Recording begins after the person begins movement. Movements are the same duration and frames as templates.

MOM uses multiple templates for the same gesture to identify gestures. The default settings record 5 templates for each gesture. Then compare a persons latest gesture to the templates.

![MOM Algorithm](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/MOM_algorithm.jpg)

Settings in Accelerometer.h control the types of gestures, the number of templates for each gesture type, the number of frames, and the template duration.

MOM uses these matching algorithms:

- Dynamic Time Warping (DTW), identifies distance between two sequences of equal length. The min() 
function finds the minimum of three values, and the distance() function calculates the Euclidean distance between two points in the sequences. The setup() function initializes Serial communication and calculates the DTW distance for two example sequences (seq1 and seq2). Finally, the DTW distance is printed to the Serial monitor.

Unused:

- Sum of averages, chooses the gesture type with the overall highest average matching ratio. Ratio must be 55% or higher to be selected.

- Highest average, chooses the gesture type with the highest single matching ratio. Ration must be 55% of higher to be selected.

MOM is extensible to include additional matching algorithms.

### Training

Training is the process of recording templates for the [Fabulous Four Gestures](#fabulous-four-gestures). Use this method to put the Gesture Sensor into training mode.

```
void setTraining( bool mode );
```

## Code

Find the implementation in the ReflectionsOS repository here: [https://github.com/frankcohen/ReflectionsOS/blob/main/src/Accellerometer.cpp](https://github.com/frankcohen/ReflectionsOS/blob/main/src/Accellerometer.cpp)

# Building

Use Arduino [IDE 2.3](https://www.arduino.cc/en/software) or later. Code requires changes to be compatible with Arduino IDE 1.x nor [Platform IO](https://platformio.org/).

Requires these libraries:

LISDHTR Accelerometer, [https://travis-ci.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR.svg?branch=master](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR.svg?branch=master)

ESP32 board, [https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)

Hardware wiring:

![Accelerometer schematic](https://github.com/frankcohen/ReflectionsOS/blob/main/Experiments/Gesture_Sensing_Accelerometer/LIS_schematic.jpg)

## Future 

We have these planned improvements to MOM:

- Template effectiveness learning. MOM recognizes and promotes templates correctly recognized, and denegrates and removes templates that do not recognize movements. It will do this by observing a person's movements after detecting a gesture. For example, when a person gestures "previous" after MOM detects a "next" gesture more than 50% of the time it is fair for MOM to identify the "next" gesture as incorrectly identified.

- Automatic new template detection. MOM recognizes new templates from movements that do not match templates. For example, when a person makes 5 "next" gestures then an unidentified new gestures then another 5 "next" gestures, MOM adds the unidentified movement gesture as a new "next" gesture.

- Template sharing between people. MOM shares templates with high success ratings among other devices.

## The Unknown

MOM has yet to be proven in these instances:

- Is there a difference in recognition between left and right handed people?

- What impact does a benign essential tremor (ET), Parkinson's disease, Zlzheimer's disease, and Multiple Sclerosis (MS) have on gesture recognition?

- Do individuals gesture differently?

## Investigations Along The Way

[@CieloStrive](https://github.com/CieloStrive) experiment on [gesture cognition using Dynamic Time Warping (DTW)](https://github.com/CieloStrive/GESTURE-RECOGNITION-DYNAMIC-TIME-WARPING) influenced this experiment. My DTW implementation is at [https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Gesture_Sensing_Accelerometer/AccelGestureDTW](https://github.com/frankcohen/ReflectionsOS/tree/main/Experiments/Gesture_Sensing_Accelerometer/AccelGestureDTW)

## ReflectionsOS Project

Reflections is an open-source hardware and software platform for building entertaining mobile experiences. Details at [https://github.com/frankcohen/ReflectionsOS](https://github.com/frankcohen/ReflectionsOS)

