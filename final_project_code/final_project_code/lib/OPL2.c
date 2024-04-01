/*
 * OPL2.c
 *
 * Created: 3/22/2024 1:52:10 PM
 *  Author: miada
 * Adapted from the ArduinoOPL2 library: https://github.com/DhrBaksteen/ArduinoOPL2/
 */ 

#include "OPL2.h"
#include "utilities.h"
#include <avr/io.h>

/*
 * Pins:
 * PC1: shift
 * PE2: latch
 * PE3: data
 * PD0: A0 (address)
 * PD1: reset
 */

#define SHIFT (1<<PORTC1) // SCLK
#define LATCH (1<<PORTE2) // SS
#define DATA  (1<<PORTE3) // MOSI
#define A0    (1<<PORTD0)
#define RESET (1<<PORTD1)

#define NUM_CHANNELS 9

// Operator definitions.
#define OPERATOR1 0
#define OPERATOR2 1
#define MODULATOR 0
#define CARRIER   1

// Synthesis type definitions.
#define SYNTH_MODE_FM 0
#define SYNTH_MODE_AM 1

// Drum sounds.
#define DRUM_BASS   0
#define DRUM_SNARE  1
#define DRUM_TOM    2
#define DRUM_CYMBAL 3
#define DRUM_HI_HAT 4

// Drum sound bit masks.
#define DRUM_BITS_BASS   0x10
#define DRUM_BITS_SNARE  0x08
#define DRUM_BITS_TOM    0x04
#define DRUM_BITS_CYMBAL 0x02
#define DRUM_BITS_HI_HAT 0x01

// Note to frequency mapping.
#define NOTE_C   0
#define NOTE_CS  1
#define NOTE_D   2
#define NOTE_DS  3
#define NOTE_E   4
#define NOTE_F   5
#define NOTE_FS  6
#define NOTE_G   7
#define NOTE_GS  8
#define NOTE_A   9
#define NOTE_AS 10
#define NOTE_B  11

// Tune specific declarations.
#define NUM_OCTAVES      7
#define NUM_NOTES       12
#define NUM_DRUM_SOUNDS  5

byte chipRegisters[3];
byte channelRegisters[27];
byte operatorRegisters[90];

const float fIntervals[8] = {
    0.048, 0.095, 0.190, 0.379, 0.759, 1.517, 3.034, 6.069
};
const unsigned int noteFNumbers[12] = {
    0x156, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA,
    0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287
};
const float blockFrequencies[8] = {
    48.503,   97.006,  194.013,  388.026,
    776.053, 1552.107, 3104.215, 6208.431
};
const byte registerOffsets[2][9] = {
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12 } ,   /*  initializers for operator 1 */
    { 0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15 }     /*  initializers for operator 2 */
};
const byte drumRegisterOffsets[2][5] = {
    { 0x10, 0xFF, 0x12, 0xFF, 0x11 },
    { 0x13, 0x14, 0xFF, 0x15, 0xFF }
};
const byte drumChannels[5] = {
    6, 7, 8, 8, 7
};
const byte drumBits[5] = {
    DRUM_BITS_BASS, DRUM_BITS_SNARE, DRUM_BITS_TOM, DRUM_BITS_CYMBAL, DRUM_BITS_HI_HAT
};

void OPL2_reset(void) {
    
}

void createShadowRegisters(void) {

}

void OPL2_init(void) {
    // set up SPI1
    DDRC  |= SHIFT;
    DDRE  |= DATA | LATCH;
    PORTC &= ~SHIFT;
    PORTE |= LATCH;
    PORTE &= ~DATA;
    SPCR1 |= (1<<SPE1) | (1<<MSTR1);
    SPSR1 |= (1<<SPI2X1);
    
    // set up other pins
    DDRD |= A0;
    DDRD |= RESET;

    PORTD &= ~A0;
    PORTD |= RESET;
    
    createShadowRegisters();
    OPL2_reset();
}