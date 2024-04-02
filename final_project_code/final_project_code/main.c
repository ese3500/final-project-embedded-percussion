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
    //lcd_init();
    OPL2_init();
    // set up timer to do pulse at tempo * 4/60 and 1 pulse on another pin every 16 steps
    // internally use these for the sequencer, but also send externally to a 3.5mm trs jack for sync signal
    sei();
}

void nextStep(void) {
    
}

const unsigned char INSTRUMENT_PIANO1[11] = { 0x00, 0x33, 0x5A, 0xB2, 0x50, 0x00, 0x31, 0x00, 0xB1, 0xF5, 0x11 };

int main(void) {
    init();
    Instrument piano = loadInstrument(INSTRUMENT_PIANO1, 0);
    setInstrument(0, piano, 1.0);
    while (1) {
        playNote(0, 4, NOTE_C);
        _delay_ms(1000);
        if (next_step) {
            nextStep();
        }        
    }
}