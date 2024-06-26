/*
 * GPIO_expander.h
 *
 * Created: 3/22/2024 3:48:23 PM
 *  Author: miada
 */ 

#ifndef GPIO_EXPANDER_H_
#define GPIO_EXPANDER_H_

#include <avr/pgmspace.h>

void GPIO_init(void);
// LED to pin numbers:
// (use LED number for argument to function)
// LED 0  -> PIN 8
// LED 1  -> PIN 9
// LED 2  -> PIN 10
// LED 3  -> PIN 11
// LED 4  -> PIN 0
// LED 5  -> PIN 1
// LED 6  -> PIN 2
// LED 7  -> PIN 3
// LED 8  -> PIN 4
// LED 9  -> PIN 5
// LED 10 -> PIN 6
// LED 11 -> PIN 7
// LED 12 -> PIN 12
// LED 13 -> PIN 13
// LED 14 -> PIN 14
// LED 15 -> PIN 15
// takes 16 bit integer and sets the leds according to the bits
void GPIO_setAllLEDs(uint16_t state);
void GPIO_setAllLEDsArray(uint8_t* state);
void GPIO_setLED(uint8_t LED, uint8_t onOff, uint8_t bright);
// some function(s) for reading the button state
uint16_t GPIO_readSteps(void);
uint16_t GPIO_readButtons(void);
uint8_t GPIO_readEncoder(void);
int32_t GPIO_readEncoderPos(void);



#endif /* GPIO_EXPANDER_H_ */