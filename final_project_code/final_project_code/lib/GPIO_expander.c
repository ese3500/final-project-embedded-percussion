/*
 * GPIO_expander.c
 *
 * Created: 3/22/2024 3:48:34 PM
 *  Author: miada
 */ 

#include "GPIO_expander.h"

void GPIO_init(void) {
    DDRC |= (1<<PORTC4) | (1<<PORTC5);
    PORTC |= (1<<PORTC4) | (1<<PORTC5);
    DDRC  &= ~((1<<PORTC4) | (1<<PORTC5));
    // 16M / (16 + 2*24) = 250k
    // change to 12 for 400k
    TWBR0 = 24;
}