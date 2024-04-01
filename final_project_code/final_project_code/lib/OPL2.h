/*
 * OPL2.h
 *
 * Created: 3/22/2024 1:52:25 PM
 *  Author: miada
 */ 

/*
 * Pins:
 * PC1: shift
 * PE2: latch
 * PE3: data
 * PD0: A0 (address)
 * PD1: reset
 */

#ifndef OPL2_H_
#define OPL2_H_

#include <avr/pgmspace.h>

typedef uint8_t byte;

typedef struct {
    int hasTremolo;
    int hasVibrato;
    int hasSustain;
    int hasEnvelopeScaling;
    byte frequencyMultiplier;
    byte keyScaleLevel;
    byte outputLevel;
    byte attack;
    byte decay;
    byte sustain;
    byte release;
    byte waveForm;
} Operator;

typedef struct {
    Operator operators[2];
    byte feedback;
    int isAdditiveSynth;
    byte transpose;
} Instrument;

void OPL2_init(void);



#endif /* OPL2_H_ */