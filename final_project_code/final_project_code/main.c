/*
 * final_project_code.c
 *
 * Created: 3/22/2024 1:50:24 PM
 * Author : Mia McMahill & Madison Hughes
 */ 
#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lib/OPL2.h"
#include "lib/ST7735.h"
#include "lib/LCD_GFX.h"

#define BD 0 // bass drum
#define SN 1 // snare
#define TM 2 // tom
#define CY 3 // cymbal
#define OH 4 // open hat
#define CH 5 // closed hat

int steps[6][16];
int tempo;
int step;
volatile int next_step;

void init(void) {
    cli();
    lcd_init();
    OPL2_init();
    // set up timer to do pulse at tempo * 4/60 and 1 pulse on another pin every 16 steps
    // internally use these for the sequencer, but also send externally to a 3.5mm trs jack for sync signal
    sei();
}

void nextStep(void) {
    
}

const unsigned char INSTRUMENT_PIANO1[11] PROGMEM = { 0x00, 0x33, 0x5A, 0xB2, 0x50, 0x00, 0x31, 0x00, 0xB1, 0xF5, 0x11 };
const unsigned char INSTRUMENT_HONKTONK[11] PROGMEM = { 0x00, 0x34, 0x9B, 0xF3, 0x63, 0x01, 0x11, 0x00, 0x92, 0xF5, 0x11 };
const unsigned char INSTRUMENT_ACOUBASS[11] PROGMEM = { 0x00, 0x21, 0x13, 0xAB, 0x46, 0x00, 0x21, 0x00, 0x93, 0xF7, 0x01 };
const unsigned char INSTRUMENT_WHISTLE[11]  PROGMEM = { 0x00, 0xE1, 0x3F, 0x9F, 0x09, 0x00, 0xE1, 0x00, 0x6F, 0x08, 0x00 };
const unsigned char INSTRUMENT_BRASS1[11]   PROGMEM = { 0x00, 0x21, 0x19, 0x87, 0x16, 0x00, 0x21, 0x03, 0x82, 0x39, 0x0F };
const unsigned char INSTRUMENT_SYNBRAS2[11] PROGMEM = { 0x00, 0x21, 0x22, 0x62, 0x58, 0x00, 0x21, 0x02, 0x72, 0x16, 0x0F };
const unsigned char INSTRUMENT_OBOE[11]     PROGMEM = { 0x00, 0x31, 0x18, 0x8F, 0x05, 0x00, 0x32, 0x01, 0x73, 0x08, 0x01 };
const unsigned char INSTRUMENT_SYNBRAS1[11] PROGMEM = { 0x00, 0x21, 0x17, 0x75, 0x35, 0x00, 0x22, 0x82, 0x84, 0x17, 0x0F };
const unsigned char INSTRUMENT_FRHORN[11]   PROGMEM = { 0x00, 0x21, 0x1F, 0x79, 0x16, 0x00, 0xA2, 0x05, 0x71, 0x59, 0x09 };

#define octave 3
#define delayamnt 200

int main(void) {
    init();
    Instrument piano = loadInstrument(INSTRUMENT_HONKTONK, 1);
    setInstrument(0, piano, 1.0);
    while (1) {
        playNote(0, octave, NOTE_C);
        _delay_ms(delayamnt);
        playNote(0, octave, NOTE_D);
        _delay_ms(delayamnt);
        playNote(0, octave, NOTE_E);
        _delay_ms(delayamnt);
        playNote(0, octave, NOTE_F);
        _delay_ms(delayamnt);
        playNote(0, octave, NOTE_G);
        _delay_ms(delayamnt);
        playNote(0, octave, NOTE_A);
        _delay_ms(delayamnt);
        playNote(0, octave, NOTE_B);
        _delay_ms(delayamnt);
        playNote(0, octave + 1, NOTE_C);
        _delay_ms(delayamnt);
        if (next_step) {
            nextStep();
        }        
    }
}