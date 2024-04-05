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
// takes 16 bit integer and sets the leds according to the bits
void GPIO_setLEDs(uint16_t state);
// some function(s) for reading the button state



#endif /* GPIO_EXPANDER_H_ */