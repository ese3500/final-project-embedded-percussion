[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/2TmiRqwI)
# final-project-skeleton

    * Team Name: Embedded Percussion
    * Team Members: Mia McMahill, Madison Hughes
    * Github Repository URL: https://github.com/ese3500/final-project-embedded-percussion
    * Github Pages Website URL: [for final submission]
    * Description of hardware: (embedded hardware, laptop, etc) 

## Final Project Proposal

### 1. Abstract

In a few sentences, describe your final project. This abstract will be used as the description in the evaluation survey forms.

### 2. Motivation

What is the problem that you are trying to solve? Why is this project interesting? What is the intended purpose?

I (Mia) love music and music technology, as well as the sound of the Yamaha FM synthesizer chips used in PC sound cards of the late 80s and early 90s. While this style of FM is not uncommon as a method for creating melodic sounds, there are very few drum machines using it to generate percussive sounds, rather than other methods such as sample playback or analog circuitry. The goal of this project is to create a drum machine with a familiar interface in the style of the Roland TR-x0x series with the sound of a Yamaha YM3812 OPL2 FM chip.

### 3. Goals

These are to help guide and direct your progress.

By the end of the project we would like to have a 16-step drum machine with 6 instrument channels and a screen for displaying settings. We plan to have 6 buttons for selecting the current channel. 16 LEDs aligned with the 16 step buttons will indicate the active steps for the current channel, as well as the current step when the sequencer is running. When no channel is selected, by pressing the button for the most recently selected channel again, the screen should display global settings including, at minimum, the tempo. Otherwise, settings for the selected channel, such as tuning and volume, should be displayed. We plan to use a rotary encoder with push button to navigate the on-screen settings.

### 4. Software Requirements Specification (SRS)

Formulate key software requirements here.

### 5. Hardware Requirements Specification (HRS)

Formulate key hardware requirements here.

### 6. MVP Demo

What do you expect to accomplish by the first milestone?

### 7. Final Demo

What do you expect to achieve by the final demonstration or after milestone 1?

### 8. Methodology

What is your approach to the problem?

First we plan to focus on connecting the buttons, LEDs, and screen, and making sure we understand using I2C to communicate with the GPIO expanders. Then we will divide up tasks for writing the sequencer firmware and a library for communicating with the OPL2 Audio Board. The interface for the OPL2 library will consist of a function that take in a pitch and a channel and trigger the sound that has been configured on the given channel at the given pitch, as well as a function that sets the volume of a channel. Additional functions to modify the sounds will be added if there is time.

### 9. Components

What major components do you need and why?

#### ATmega328PB

We plan to use an ATmega328PB as the brains of our device as we have the most familiarity with it, and its specifications match our requirements: 2 SPI buses, I2C bus, and 16 bit timers.

#### Adafruit 358 Screen

We need the current tempo to be displayed, and would like to potentially display other information. We plan to use this screen as we already have it.

#### OPL2 Audio Board

A drum machine requires some method of producing audio. The OPL2 was a widely used chip in PC sound cards and Yamaha keyboards, so it's use for generating musical sounds is well established. It provides 9 instrument channels that can be used simultaneously, fitting our goal of having at least 6. This board provides a serial interface for the chip, as well as incorporating the necessary DAC and timing circuitry required to use it.

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
| Week 1: 3/24 - 3/31 |          |                    |
| Week 2: 4/1 - 4/7   |          |                    |
| Week 3: 4/8 - 4/14  |          |                    |
| Week 4: 4/15 - 4/21 |          |                    |
| Week 5: 4/22 - 4/26 |          |                    |

### 12. Proposal Presentation

Add your slides to the Final Project Proposal slide deck in the Google Drive.

## Final Project Report

Don't forget to make the GitHub pages public website!
If you’ve never made a Github pages website before, you can follow this webpage (though, substitute your final project repository for the Github username one in the quickstart guide):  <https://docs.github.com/en/pages/quickstart>

### 1. Video

[Insert final project video here]

### 2. Images

[Insert final project images here]

### 3. Results

What were your results? Namely, what was the final solution/design to your problem?

#### 3.1 Software Requirements Specification (SRS) Results

Based on your quantified system performance, comment on how you achieved or fell short of your expected software requirements. You should be quantifying this, using measurement tools to collect data.

#### 3.2 Hardware Requirements Specification (HRS) Results

Based on your quantified system performance, comment on how you achieved or fell short of your expected hardware requirements. You should be quantifying this, using measurement tools to collect data.

### 4. Conclusion

Reflect on your project. Some questions to consider: What did you learn from it? What went well? What accomplishments are you proud of? What did you learn/gain from this experience? Did you have to change your approach? What could have been done differently? Did you encounter obstacles that you didn’t anticipate? What could be a next step for this project?

## References

Fill in your references here as you work on your proposal and final submission. Describe any libraries used here.

## Github Repo Submission Resources

You can remove this section if you don't need these references.

* [ESE5160 Example Repo Submission](https://github.com/ese5160/example-repository-submission)
* [Markdown Guide: Basic Syntax](https://www.markdownguide.org/basic-syntax/)
* [Adobe free video to gif converter](https://www.adobe.com/express/feature/video/convert/video-to-gif)
* [Curated list of example READMEs](https://github.com/matiassingers/awesome-readme)
* [VS Code](https://code.visualstudio.com/) is heavily recommended to develop code and handle Git commits
  * Code formatting and extension recommendation files come with this repository.
  * Ctrl+Shift+V will render the README.md (maybe not the images though)
