/*
 * final_project_code.c
 *
 * Created: 3/22/2024 1:50:24 PM
 * Author : Mia McMahill & Madison Hughes
 */ 
#define F_CPU 16000000

#include <avr/io.h>
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
    //lcd_init();
    OPL2_init();
    // set up timer to do pulse at tempo * 4/60 and 1 pulse on another pin every 16 steps
    // internally use these for the sequencer, but also send externally to a 3.5mm trs jack for sync signal
}

void nextStep(void) {
    
}

int main(void) {
    init();
    while (1) {
        if (next_step) {
            nextStep();
        }        
    }
}

