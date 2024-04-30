# Final Project: FM Drum Machine

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/2TmiRqwI)

    * Team Name: Embedded Percussion
    * Team Members: Mia McMahill, Madison Hughes
    * Github Repository URL: https://github.com/ese3500/final-project-embedded-percussion
    * Github Pages Website URL: https://ese3500.github.io/final-project-embedded-percussion/
    * Description of hardware: Laptops (Win 10, Win 11), ATmega328PB Xplained Mini, Adafruit AW9523 GPIO Expanders, OPL2 Audio Board, Adafruit I2C Rotary Encoder Board

## Final Project Proposal

### 1. Abstract

In a few sentences, describe your final project. This abstract will be used as the description in the evaluation survey forms.

This project is a drum machine with a 16-step sequencer and 6 instruments. The sequencer interface resembles that of Roland TR-x0x style drum machines, and the sound is generated by the Yamaha YM3812 FM chip. Additionally, it has a screen for managing settings such as tempo, volume, and tuning.

### 2. Motivation

What is the problem that you are trying to solve? Why is this project interesting? What is the intended purpose?

I (Mia) love music and music technology, as well as the sound of the Yamaha FM synthesizer chips used in PC sound cards of the late 80s and early 90s. While this style of FM is not uncommon as a method for creating melodic sounds, there are very few drum machines using it to generate percussive sounds, rather than other methods such as sample playback or analog circuitry. The goal of this project is to create a drum machine with a familiar interface in the style of the Roland TR-x0x series with the sound of a Yamaha YM3812 FM chip.

### 3. Goals

These are to help guide and direct your progress.

By the end of the project we would like to have a 16-step drum machine with 6 instrument channels and a screen for displaying settings. We plan to have 6 buttons for selecting the current channel. 16 LEDs aligned with the 16 step buttons will indicate the active steps for the current channel, as well as the current step when the sequencer is running. When no channel is selected, by pressing the button for the most recently selected channel again, the screen should display global settings including, at minimum, the tempo. Otherwise, settings for the selected channel, such as tuning and volume, should be displayed. We plan to use a rotary encoder with push button to navigate the on-screen settings.

### 4. Software Requirements Specification (SRS)

Formulate key software requirements here.

#### Overview

Using I2C and SPI, the ATmega328PB shall interface with the OPL2 Audio Board, GPIO expanders, and LCD screen. The software shall keep track of active steps in a sequence, looping over the 16 steps and triggering sounds to be generated by the YM3812 in accordance with the active steps. It shall allow users to modify tempo of the sequencer, the active steps, and tuning and volume of the 6 instruments. At startup, it shall configure sounds on channels 1-6 of the YM3812.

#### Users

The users for this device are musicians who want to create 16-step looping drum tracks.

#### Definitions/Abbreviations

step &mdash; units representing the sixteen 16th notes of a 4/4 measure

active step &mdash; For each of the 6 instrument channels, a step may be active or inactive. If if a step is active for a channel, when the sequencer reaches that step, the sound of that channel will be played.

#### Functionality

SRS 01 &mdash; When no channel is selected, users shall be able to use an on-screen menu to set a tempo in a range of 60-300 BPM.

SRS 02 &mdash; The software shall use a PWM timer with a frequency of $`\lfloor\frac{16000000}{2*128*(tempo * 4 / 60)}\rfloor`$. A pin shall be driven high or low in accordance with this pulse signal.

SRS 03 &mdash; When the sequencer is running, for each step, for each channel for which that step is active, a sound corresponding to the channel shall be played by the YM3812.

SRS 04 &mdash; Users shall be able to start or stop the sequencer at any time. Playback shall always start from the first step whenever the sequencer is started.

SRS 05 &mdash; Users shall be able to change the current instrument channel as well as set the active steps for that channel at any time.

SRS 06 &mdash; Using an on-screen menu, users shall be able to modify the tuning and volume of the current instrument channel.

### 5. Hardware Requirements Specification (HRS)

Formulate key hardware requirements here.

#### Overview

The hardware of the design will consist of a series of buttons with various purposes interfacing with the microcontroller, a screen and the synthesizer.

#### Definitions/Abbreviations

OPL2 - Operator Type L 2, another name for the YM3812

#### Functionality

HRS 01 &mdash; ATmega328PB will be the main microcontroller for this design\
HRS 02 &mdash; An adafruit GPIO expander to expand the amount of available pins/registers we have access to\
HRS 03 &mdash; 16 pushbuttons to represent different notes in a measure\
HRS 04 &mdash; 6 more pushbuttons to represent types of drums\
HRS 05 &mdash; 1.8” 128x160 TFT LCD module to display the tempo, volume and the current drum setting\
HRS 06 &mdash; OPL2 Audio Board using a Yamaha YM3812 sound chip with SPI interfacing for audio synthesizing\
HRS 07 &mdash; A rotary encoder for controlling the screen menus and the volume

### 6. MVP Demo

By the first demo, we aim to have all of our features implemented. This way we can leave the fine tuning and the polishing for the last week. This means all of the software and hardware will be implemented and complete, including but not limited to all buttons, knobs and switches. Any extra features we many have time for may or may not be completely implemented at this time

### 7. Final Demo

For the final demo, we will have our full project. This means that since the first demo we will have the entire casing for our design completed (will be 3d printed). The project will be in its full and polished versions. Any extra features that we may have time for will also be implemented during this time and at this point.

### 8. Methodology

What is your approach to the problem?

First we plan to focus on connecting the buttons, LEDs, and screen, and making sure we understand using I2C to communicate with the GPIO expanders. Then we will divide up tasks for writing the sequencer firmware and a library for communicating with the OPL2 Audio Board. The interface for the YM3812 library will consist of a function that take in a pitch and a channel and trigger the sound that has been configured on the given channel at the given pitch, as well as a function that sets the volume of a channel. Additional functions to modify the sounds will be added if there is time.

### 9. Components

What major components do you need and why?

#### ATmega328PB

We plan to use an ATmega328PB as the brains of our device as we have the most familiarity with it, and its specifications match our requirements: 2 SPI buses, I2C bus, and 16 bit timers.

#### Adafruit 358 Screen

We need the current tempo to be displayed, and would like to potentially display other information. We plan to use this screen as we already have it.

#### OPL2 Audio Board

A drum machine requires some method of producing audio. The YM3812 was a widely used chip in PC sound cards and Yamaha keyboards, so it's use for generating musical sounds is well established. It provides 9 instrument channels that can be used simultaneously, fitting our goal of having at least 6. This board provides a serial interface for the chip, as well as incorporating the necessary DAC and timing circuitry required to use it.

#### 16 Step Switch (Adafruit 5499)

Most step sequencers use a row of 16 buttons and LEDs, with the LEDs used to indicate the active steps. Adafruit's step switches conveniently contain both a push button and an led in one casing, providing both components as well as being aesthetically pleasing.

#### GPIO Expander

This project will require many buttons (~25) and LEDs (16). The ATmega does not have enough pins to directly connect each of these, or to wire them in a matrix, so by using a pair of I2C addressable GPIO expanders, we will be able to have enough pins to wire all our peripherals to the MCU.

### 10. Evaluation

What is your metric for evaluating how well your product/solution solves the problem? Think critically on this section. Having a boolean metric such as “it works” is not very useful. This is akin to making a speaker and if it emits sound, albeit however terrible and ear wrenching, declare this a success.
It is recommended that your project be something that you can take pride in. Oftentimes in interviews, you will be asked to talk about projects you have worked on.

Our metric will be made up of the 2 halves below:

#### Sequencer

For the sequencer aspect of this project, the interface (buttons, LEDs, and on-screen menus) should be responsive, with no noticeable delay between input and updating the displays. The tempo should be adjustable in a range of ~60-300 bpm. The tempo should not be inaccurate, and the adjustment should not be too coarse (increments of >4). When it comes to programming the steps and starting and stopping the sequencer, either it works or it doesn't.

#### Drum Sounds/YM3812 Interfacing

The minimum for this part to be considered working at all is for some type of sound to be outputted in accordance with the sequenced step pattern. To meet our goals, each of the sounds should be tunable and have adjustable volume. More subjectively, the sounds should be pleasing and resemble actual drums (bass, snare, hi-hat, etc.).

### 11. Timeline

This section is to help guide your progress over the next few weeks. Feel free to adjust and edit the table below to something that would be useful to you. Really think about what you want to accomplish by the first milestone.

| **Week**            | **Task** | **Assigned To**    |
|----------           |--------- |------------------- |
| Week 1: 3/24 - 3/31 | finishing design and buying parts   | Madison and Mia |
| Week 2: 4/1 - 4/7   | assemble OPL2 board                       |  Madison  |
|                     | OPL2 library                              |  Mia      |
| Week 3: 4/8 - 4/14  | button and led gpio expander interfacing  |  Madison  |
|                     | write main sequencer code                 |  Mia      |
| Week 4: 4/15 - 4/21 | rotary encoder and complete functionality testing | Madison |
|                     | on-screen UI                              |  Mia      |
| Week 5: 4/22 - 4/26 | case design and production                |  Madison  |
|                     | software bugfixing and polish UI          |  Mia      |

### 12. Proposal Presentation

Add your slides to the Final Project Proposal slide deck in the Google Drive.

## Final Project Report

Don't forget to make the GitHub pages public website!
If you’ve never made a Github pages website before, you can follow this webpage (though, substitute your final project repository for the Github username one in the quickstart guide):  <https://docs.github.com/en/pages/quickstart>

### 1. Video

On [YouTube](https://youtu.be/iLOzhloqFKE) (placeholder til recording final video)

### 2. Images

On [Github Pages](https://ese3500.github.io/final-project-embedded-percussion/)

### 3. Results

What were your results? Namely, what was the final solution/design to your problem?

Our final design was what we had envisioned from the start. It is a laser-cut acrylic enclosure containing the electronics, with 23 buttons, a rotary encoder, a screen, sync jack, and the OPL2 accessible from the top panel. 16 of the buttons have integrated LEDs to display information about the drum pattern and playback, while the other 7 are unlit. It allows for the creation and playback of 6 channel, 16-step drum sequences, with the ability to modify 4 per channel settings, as well as global tempo control.

#### 3.1 Software Requirements Specification (SRS) Results

Based on your quantified system performance, comment on how you achieved or fell short of your expected software requirements. You should be quantifying this, using measurement tools to collect data.

<span style="color:lightgreen">SRS 01</span> &mdash; When no channel is selected, users shall be able to use an on-screen menu to set a tempo in a range of 60-300 BPM.

<span style="color:lightgreen">SRS 02</span> &mdash; The software shall use a PWM timer with a frequency of $`\lfloor\frac{8000000}{2*64*(tempo * 4 / 60)}\rfloor`$. A pin shall be driven high or low in accordance with this pulse signal.

<span style="color:lightgreen">SRS 03</span> &mdash; When the sequencer is running, for each step, for each channel for which that step is active, a sound corresponding to the channel shall be played by the YM3812.

<span style="color:lightgreen">SRS 04</span> &mdash; Users shall be able to start or stop the sequencer at any time. Playback shall always start from the first step whenever the sequencer is started.

<span style="color:lightgreen">SRS 05</span> &mdash; Users shall be able to change the current instrument channel as well as set the active steps for that channel at any time.

<span style="color:lightgreen">SRS 06</span> &mdash; Using an on-screen menu, users shall be able to modify the tuning and volume of the current instrument channel.

Our final design meets all software requirements. Our requirements changed slightly over time due to the timing needs of the serial interfaces and the sequencer. SRS02 was updated accordingly. Additionally, we made it possible to modify tempo at all times, rather than only when no instrument channel is selected (it is also not possible to not have one selected), slightly differing from SRS01.

Testing the majority of the requirements were tested just by using the device, as whether or not they are met is easily perceived without measurement tools.

The frequency of the signal on the sync port was measured using an oscilloscope to test SRS02.
![scope](docs/images/scope_srs02.jpg)

#### 3.2 Hardware Requirements Specification (HRS) Results

Based on your quantified system performance, comment on how you achieved or fell short of your expected hardware requirements. You should be quantifying this, using measurement tools to collect data.

<span style="color:lightgreen">HRS 01</span> &mdash; ATmega328PB will be the main microcontroller for this design

<span style="color:lightgreen">HRS 02</span> &mdash; An adafruit GPIO expander to expand the amount of available pins/registers we have access to

<span style="color:lightgreen">HRS 03</span> &mdash; 16 pushbuttons to represent different notes in a measure

<span style="color:lightgreen">HRS 04</span> &mdash; 6 more pushbuttons to represent types of drums

<span style="color:lightgreen">HRS 05</span> &mdash; 1.8” 128x160 TFT LCD module to display the tempo, volume and the current drum setting

<span style="color:lightgreen">HRS 06</span> &mdash; OPL2 Audio Board using a Yamaha YM3812 sound chip with SPI interfacing for audio synthesizing

<span style="color:lightgreen">HRS 07</span> &mdash; A rotary encoder for controlling the screen menus and the volume

Our final design meets all hardware requirements. No changes were made to our hardware requirements over the course of the project.

### 4. Conclusion

Reflect on your project. Some questions to consider: What did you learn from it? What went well? What accomplishments are you proud of? What did you learn/gain from this experience? Did you have to change your approach? What could have been done differently? Did you encounter obstacles that you didn’t anticipate? What could be a next step for this project?

<b>Mia:</b> I think the primary think I learned is the importance of spare components. We had one of our GPIO expanders, and many of the LEDs in our buttons, break, and we were saved by the parts being ordered twice by the Detkin staff. When our encoder broke, and we had to wait after putting in a new order, it delayed getting that aspect finished until a few days before the final demo. Other than some issues with broken components, just about everything went well. In my opinion, we achieved exactly what we set out to do, with the final device having all planned features and functioning very reliably. One unexpected (well, somewhat expected) issue was that the slowness of drawing characters to the LCD necessitated a minor change to the design of the GUI. Instead of changing it entirely for the tempo selection, I added the tempo to be displayed on the per-instrument screens, and removed the global settings screen. This was a simple change though that didn't affect anything else. With the fact that everything works so reliably and the number wires are becoming unmanageable, I think a good next step (that I plan on taking) would be to design a PCB to make the hardware more compact and easy to manage.

<b>Madison:</b>

## References

Fill in your references here as you work on your proposal and final submission. Describe any libraries used here.

Adapted the Arduino OPL2 library to bare metal C:\
<https://github.com/DhrBaksteen/ArduinoOPL2>\
Referenced Adafruit Seesaw library when implementing the encoder functionality:\
<https://github.com/adafruit/Adafruit_Seesaw/tree/master>

## Github Repo Submission Resources

You can remove this section if you don't need these references.

* [ESE5160 Example Repo Submission](https://github.com/ese5160/example-repository-submission)
* [Markdown Guide: Basic Syntax](https://www.markdownguide.org/basic-syntax/)
* [Adobe free video to gif converter](https://www.adobe.com/express/feature/video/convert/video-to-gif)
* [Curated list of example READMEs](https://github.com/matiassingers/awesome-readme)
* [VS Code](https://code.visualstudio.com/) is heavily recommended to develop code and handle Git commits
  * Code formatting and extension recommendation files come with this repository.
  * Ctrl+Shift+V will render the README.md (maybe not the images though)
