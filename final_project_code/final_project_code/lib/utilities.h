/*
 * utilities.h
 *
 * Created: 4/1/2024 12:35:33 PM
 *  Author: miada
 */ 

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <util/delay.h>

#define set(reg,bit) (reg) |= (1<<(bit))
#define clear(reg,bit) (reg) &= ~(1<<(bit))
#define toggle(reg,bit) (reg) ^= (1<<(bit))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
//#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#define CLAMP(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/**************************************************************************//**
* @fn            void Delay_ms(unsigned int n)
* @brief        Delay function using variables
* @note
*****************************************************************************/
void Delay_ms(unsigned int n);

#endif /* UTILITIES_H_ */