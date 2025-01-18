#include <PeakDetection.h>

PeakDetection peakDetection;

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  peakDetection.begin(48, 3, 0.6);
}

void loop() {
    double data = (double)analogRead(A0)/512-1;
    peakDetection.add(data);
    int peak = peakDetection.getPeak();
    double filtered = peakDetection.getFilt();
    Serial.print(data);
    Serial.print(",");
    Serial.print(peak);
    Serial.print(",");
    Serial.println(filtered);
}