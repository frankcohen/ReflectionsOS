
# Stream Analog Input to Serial Output

## General Description:

We just send the audio data that we receive from microphone the Serial output, so that we can monitor it in the Serial Plotter.

Here is the result on the Arduino Serial Plotter:

![serial-plotter](https://pschatzmann.github.io/Resources/img/serial-plotter-sine.png)


### Analog Input:

| ADC     |  ESP32
| --------| ---------------
| VCC     |  3.3V
| GND     |  GND
| OUT     |  GPIO34

For the input I was using a MCP6022 Microphone Sensor.
Plaese note that the signal that we receive from the ADC needs to be adjusted so that it is oscillating around 0.

![MCP6022](https://pschatzmann.github.io/Resources/img/mcp6022.jpeg)
