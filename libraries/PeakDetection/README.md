# PeakDetection

**PeakDetection** is an Arduino library for real time peak detection in sensor data.

[🇫🇷 Traduire](https://github.com/leandcesar/PeakDetection/blob/master/locale/README.fr.md) | [🇧🇷 Traduzir](https://github.com/leandcesar/PeakDetection/blob/master/locale/README.pt-BR.md)

## Algorithm

It is based on the [principle of dispersion](https://en.wikipedia.org/wiki/Statistical_dispersion): if a new datapoint is a given `x` number of [standard deviations](https://en.wikipedia.org/wiki/Standard_deviation) away from some [moving average](https://en.wikipedia.org/wiki/Moving_average), the algorithm signals (also called [z-score](https://en.wikipedia.org/wiki/Standard_score)).

The algorithm takes 3 inputs:

* `lag`: is the lag of the moving window. This parameter determines how much your data will be smoothed and how adaptive the algorithm is to changes in the long-term average of the data. The more stationary your data is, the more lags you should include. If your data contains time-varying trends, you should consider how quickly you want the algorithm to adapt to these trends.

* `threshold`: this parameter is the number of standard deviations from the moving mean above which the algorithm will classify a new datapoint as being a signal. This parameter should be set based on how many signals you expect. The threshold therefore directly influences how sensitive the algorithm is and thereby also how often the algorithm signals.

* `influence`: is the z-score at which the algorithm signals. This parameter determines the influence of signals on the algorithm's detection threshold. If put at 0, signals have no influence on the threshold, such that future signals are detected based on a threshold that is calculated with a mean and standard deviation that is not influenced by past signals. You should put the influence parameter somewhere between 0 and 1, depending on the extent to which signals can systematically influence the time-varying trend of the data.

## Library

* `begin()`: initialize `PeakDetection` object and configuration for tweaking parameters. If no parameters have been set, the default remains. Parameters: `lag` (default=32), `threshold` (default=2), `influence` (default=0.5).

* `add()`: adds a new data point to algorithm, calculates the standard deviations and moving average.

* `getPeak()`: returns peak status of the last data point added. {-1, 0, 1}, representing below, within or above standard deviation threshold, respectively.

* `getFilt()`: returns last data point filtered by the moving average.

## Installation

To use the library:

1. Download the zip and uncompress the downloaded file.
2. Copy the folder to the Arduino libraries folder (`C:/Users/username/Documents/Arduino/libraries`).
3. Rename it to `PeakDetection`.

## Example

```C++
#include <PeakDetection.h>                       // import lib

PeakDetection peakDetection;                     // create PeakDetection object

void setup() {
  Serial.begin(9600);                            // set the data rate for the Serial communication
  pinMode(A0, INPUT);                            // analog pin used to connect the sensor
  peakDetection.begin(48, 3, 0.6);               // sets the lag, threshold and influence
}

void loop() {
    double data = (double)analogRead(A0)/512-1;  // converts the sensor value to a range between -1 and 1
    peakDetection.add(data);                     // adds a new data point
    int peak = peakDetection.getPeak();          // 0, 1 or -1
    double filtered = peakDetection.getFilt();   // moving average
    Serial.print(data);                          // print data
    Serial.print(",");
    Serial.print(peak);                          // print peak status
    Serial.print(",");
    Serial.println(filtered);                    // print moving average
}
```

![Example](https://github.com/leandcesar/PeakDetection/blob/master/docs/assets/example.gif)

## Acknowledgments

* [StackOverFlow](https://stackoverflow.com/questions/22583391/peak-signal-detection-in-realtime-timeseries-data)
