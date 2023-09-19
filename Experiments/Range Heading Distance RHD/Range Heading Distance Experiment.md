A solution to determine range, heading, and distance between devices, ESP32-S3, BlE, and magnetometer/compass

## The problem

How do mobile devices know if they are in range and heading towards each other when GPS is not available? When the device is equipped with a compass (magnetometer) a clever use of trigonometry I call Range Heading Distance (RHD) is 78% accurate.

This technique works with any processor and compass (magnetometer)


## The solution

## Why it seems more accurate than it is?


Trigonometry

Diagrams of triangles to determine heading

Assumptions and Where it doesn't work


Code

Get the RSSI 

