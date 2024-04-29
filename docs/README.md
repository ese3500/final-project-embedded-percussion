---
layout: default
---
# Final Report

## Video

{% include youtube.html id="S4be1K6JF78" %}

<br>

## Final Project Images

||||
|-|-|-|
| ![front view](images/front.jpg) | ![top view](images/top.jpg) | ![back view](images/back.jpg) |
| ![side1 view](images/side1.jpg) | ![side2 view](images/side2.jpg) | ![corner1 view](images/corner1.jpg) |
| ![corner3 view](images/corner3.jpg) | ![corner4 view](images/corner4.jpg) | ![corner2 view](images/corner2.jpg)  |
| ![powered on view](images/powered_on.jpg) | ![screen view](images/screen.jpg) | |

## Progress Photos

## Results

## Specifications Results

### SRS

<span style="color:lightgreen">SRS 01</span> &mdash; When no channel is selected, users shall be able to use an on-screen menu to set a tempo in a range of 60-300 BPM.

<span style="color:lightgreen">SRS 02</span> &mdash; The software shall use a PWM timer with a frequency of $`\lfloor\frac{16000000}{2*128*(tempo * 4 / 60)}\rfloor`$. A pin shall be driven high or low in accordance with this pulse signal.

<span style="color:lightgreen">SRS 03</span> &mdash; When the sequencer is running, for each step, for each channel for which that step is active, a sound corresponding to the channel shall be played by the YM3812.

<span style="color:lightgreen">SRS 04</span> &mdash; Users shall be able to start or stop the sequencer at any time. Playback shall always start from the first step whenever the sequencer is started.

<span style="color:lightgreen">SRS 05</span> &mdash; Users shall be able to change the current instrument channel as well as set the active steps for that channel at any time.

<span style="color:lightgreen">SRS 06</span> &mdash; Using an on-screen menu, users shall be able to modify the tuning and volume of the current instrument channel.

Our final design meets all software requirements.

### HRS

<span style="color:lightgreen">HRS 01</span> &mdash; ATmega328PB will be the main microcontroller for this design

<span style="color:lightgreen">HRS 02</span> &mdash; An adafruit GPIO expander to expand the amount of available pins/registers we have access to

<span style="color:lightgreen">HRS 03</span> &mdash; 16 pushbuttons to represent different notes in a measure

<span style="color:lightgreen">HRS 04</span> &mdash; 6 more pushbuttons to represent types of drums

<span style="color:lightgreen">HRS 05</span> &mdash; 1.8” 128x160 TFT LCD module to display the tempo, volume and the current drum setting

<span style="color:lightgreen">HRS 06</span> &mdash; OPL2 Audio Board using a Yamaha YM3812 sound chip with SPI interfacing for audio synthesizing

<span style="color:lightgreen">HRS 07</span> &mdash; A rotary encoder for controlling the screen menus and the volume

Our final design meets all hardware requirements.

## Conclusion

## References

Adapted the Arduino OPL2 library to bare metal C:\
<https://github.com/DhrBaksteen/ArduinoOPL2>\
Referenced Adafruit Seesaw library when implementing the encoder functionality:\
<https://github.com/adafruit/Adafruit_Seesaw/tree/master>
