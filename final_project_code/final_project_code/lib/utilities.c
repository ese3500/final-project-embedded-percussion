/*
 * utilities.c
 *
 * Created: 4/1/2024 1:13:48 PM
 *  Author: miada
 */ 

#include "utilities.h"
#include <util/delay.h>

void Delay_ms(unsigned int n) {
     while (n--) {
         _delay_ms(1);
     }
 }