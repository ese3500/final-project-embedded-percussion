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
#include "lib/GPIO_expander.h"

#define BD 0 // bass drum
#define SN 1 // snare
#define CL 2 // clap
#define CY 3 // cymbal
#define OH 4 // open hat
#define CH 5 // closed hat

#define NUM_INST 6

int steps[6][16] = {
    {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}        
};
const uint8_t octaves[6] = {1, 2, 3, 4, 4, 4};
int tempo;
int step;
int evenorodd = 0;
volatile int next_step;
volatile int input_intrpt1;
volatile int input_intrpt2;

const unsigned char INSTRUMENT_PIANO1[11] PROGMEM = { 0x00, 0x33, 0x5A, 0xB2, 0x50, 0x00, 0x31, 0x00, 0xB1, 0xF5, 0x11 };
const unsigned char INSTRUMENT_HONKTONK[11] PROGMEM = { 0x00, 0x34, 0x9B, 0xF3, 0x63, 0x01, 0x11, 0x00, 0x92, 0xF5, 0x11 };
const unsigned char INSTRUMENT_ACOUBASS[11] PROGMEM = { 0x00, 0x21, 0x13, 0xAB, 0x46, 0x00, 0x21, 0x00, 0x93, 0xF7, 0x01 };
const unsigned char INSTRUMENT_WHISTLE[11]  PROGMEM = { 0x00, 0xE1, 0x3F, 0x9F, 0x09, 0x00, 0xE1, 0x00, 0x6F, 0x08, 0x00 };
const unsigned char INSTRUMENT_BRASS1[11]   PROGMEM = { 0x00, 0x21, 0x19, 0x87, 0x16, 0x00, 0x21, 0x03, 0x82, 0x39, 0x0F };
const unsigned char INSTRUMENT_SYNBRAS2[11] PROGMEM = { 0x00, 0x21, 0x22, 0x62, 0x58, 0x00, 0x21, 0x02, 0x72, 0x16, 0x0F };
const unsigned char INSTRUMENT_OBOE[11]     PROGMEM = { 0x00, 0x31, 0x18, 0x8F, 0x05, 0x00, 0x32, 0x01, 0x73, 0x08, 0x01 };
const unsigned char INSTRUMENT_SYNBRAS1[11] PROGMEM = { 0x00, 0x21, 0x17, 0x75, 0x35, 0x00, 0x22, 0x82, 0x84, 0x17, 0x0F };
const unsigned char INSTRUMENT_FRHORN[11]   PROGMEM = { 0x00, 0x21, 0x1F, 0x79, 0x16, 0x00, 0xA2, 0x05, 0x71, 0x59, 0x09 };
const unsigned char DRUMINS_BASS_DR1[11]  PROGMEM = { 0x30, 0x01, 0x07, 0xFA, 0xFD, 0x00, 0x01, 0x00, 0xF6, 0x47, 0x05 };
const unsigned char DRUMINS_SNARE_AC[11]  PROGMEM = { 0x30, 0x24, 0x00, 0xFF, 0x0F, 0x00, 0x02, 0x00, 0xF7, 0xA9, 0x0F };
const unsigned char DRUMINS_SNARE_EL[11]  PROGMEM = { 0x30, 0x24, 0x00, 0xFF, 0x0F, 0x00, 0x02, 0x00, 0xF7, 0xA9, 0x0F };
const unsigned char DRUMINS_HIHAT_CL[11]  PROGMEM = { 0x30, 0x2C, 0x00, 0xF2, 0xFE, 0x00, 0x02, 0x06, 0xB8, 0xD8, 0x37 };
const unsigned char DRUMINS_HIHAT_OP[11]  PROGMEM = { 0x30, 0x2E, 0x00, 0x82, 0xF6, 0x00, 0x04, 0x10, 0x74, 0xF8, 0x35 };
const unsigned char DRUMINS_CLAP[11]      PROGMEM = { 0x30, 0x3E, 0x00, 0x9F, 0x0F, 0x00, 0x30, 0x00, 0x87, 0xFA, 0x0F };
const unsigned char DRUMINS_LO_TOMS[11]   PROGMEM = { 0x30, 0x06, 0x0A, 0xFA, 0x1F, 0x00, 0x11, 0x00, 0xF5, 0xF5, 0x0C };

ISR(PCINT3_vect) {
    input_intrpt1 = 1;
}

void init(void) {
    cli();
    DDRE &= ~(1<<PORTE0);
    //PORTE |= (1<<PORTE0);
    PCICR |= 1<<PCIE3;
    // pe0 and pe1 pin change interrupts
    PCMSK3 |= 1<<PCINT24;
    //PCMSK3 |= 1<<PCINT25;
    lcd_init();
    OPL2_init();
    GPIO_init();
    // set up timer to do pulse at tempo * 4/60 and 1 pulse on another pin every 16 steps
    // internally use these for the sequencer, but also send externally to a 3.5mm trs jack for sync signal
    sei();
}

void nextStep(void) {
    // play all the notes for this step
    for (int i = 0; i < NUM_INST; i++) {
        if (steps[i][step]) {
            playNote(i, octaves[i], NOTE_C);
        }
    }
    // TODO: change this to set to if the step is active, not set it to off
    GPIO_setLED(step == 0 ? 15 : step - 1, 0);
    GPIO_setLED(step, 1);
    next_step = 0;
    step = (step + 1) % 16;
}

void setUpInstruments(void) {
    Instrument bass_drum = loadInstrument(DRUMINS_BASS_DR1, 1);
    setInstrument(BD, bass_drum, 1.0);
    
    Instrument snare_drum = loadInstrument(DRUMINS_SNARE_EL, 1);
    setInstrument(SN, snare_drum, 1.0);
    
    Instrument clap = loadInstrument(DRUMINS_CLAP, 1);
    setInstrument(CL, clap, 1.0);
    
    Instrument cymbal = loadInstrument(DRUMINS_LO_TOMS, 1);
    setInstrument(CY, cymbal, 1.0);
    
    Instrument open_hat = loadInstrument(DRUMINS_HIHAT_OP, 1);
    setInstrument(OH, open_hat, 1.0);
    
    Instrument closed_hat = loadInstrument(DRUMINS_HIHAT_CL, 1);
    setInstrument(CH, closed_hat, 1.0);
}

void handle_step_input(void) {
    uint16_t step_buttons = GPIO_readSteps();
    input_intrpt1 = 0;
}

void handle_button_input(void) {
    uint16_t buttons = GPIO_readButtons();
    input_intrpt2 = 0;
}

int main(void) {
    init();
    setUpInstruments();
    LCD_setScreen(BLACK);
    LCD_drawString(22, 22, "Drum Machine", WHITE, BLACK);
    while (1) {
        if (next_step) {
          nextStep();
          continue;
        }
        if (input_intrpt1) {
            next_step = 1;
            handle_step_input();
        }
    }
}