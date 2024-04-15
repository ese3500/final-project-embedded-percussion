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
Instrument createInstrument();
Instrument loadInstrument(const unsigned char *instrumentData, int fromProgmem);
void setInstrument(byte channel, Instrument instrument, float volume);
void setBlock(byte channel, byte block);
void playNote(byte channel, byte octave, byte note);
void setChannelVolume(byte channel, byte volume);


#endif /* OPL2_H_ */