/*
 * final_project_code.c
 *
 * Created: 3/22/2024 1:50:24 PM
 * Author : Mia McMahill & Madison Hughes
 */ 
#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
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

#define PAR_A 0
#define PAR_B 1
#define TUN   2
#define VOL   3
#define TEMPO 4

#define B_MASK 0x7F
#define B_BD   0x1
#define B_SN   0x2
#define B_CL   0x4
#define B_CY   0x8
#define B_OH   0x10
#define B_CH   0x20
#define B_STRT 0x40

// TODO: find correct values for these
#define ENC_MASK  0x07
#define ENC_UP    0x01
#define ENC_DOWN  0x02
#define ENC_PRESS 0x04

#define NUM_INST 6

uint8_t steps[6][16] = {
    {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}, // BD
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, // SN
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0}, // CL
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // CY
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0}, // OH
    {0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0}  // CH 
};
//uint8_t steps[6][16];
uint8_t settings[6][4] = {
    // A,  B,  TUN, VOL
    {127, 127, 127, 255},
    {127, 127, 127, 255},
    {127, 127, 127, 255},
    {127, 127, 127, 255},
    {127, 127, 127, 255},
    {127, 127, 127, 255}
};
const uint8_t octaves[6] = {1, 2, 3, 2, 2, 2};
int tempo = 96;
char tempoStr[5];
int step = 0;
int current_channel = BD;
int stopped = 0;
int select_mode = 1;
int current_setting = 0;
volatile int send_start = 0;
volatile int ignore_next = 0;
volatile int next_step = 0;
volatile int input_intrpt1 = 0;
volatile int input_intrpt2 = 0;
volatile int encoder_intrpt = 0;

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
    if (!(PINE & (1<<PINE0))) {
        input_intrpt1 = 1;
    }
    if (!(PINE & (1<<PINE1))) {
        input_intrpt2 = 1;
    }
}


ISR(TIMER3_COMPA_vect) {
    next_step = !stopped;
    PORTD |= (!stopped)<<PORTD3;
    PORTD |= (send_start<<PORTD2);
}

ISR(TIMER3_COMPB_vect) {
    PORTD &= ~((1<<PORTD3) | (1<<PORTD2));
    send_start = 0;
}

void init(void) {
    cli();
    DDRE &= ~((1<<DDE0) | (1<<DDE1));
    //PORTE |= (1<<PORTE0);
    PCICR |= 1<<PCIE3;
    // pe0 and pe1 pin change interrupts
    PCMSK3 |= 1<<PCINT24;
    PCMSK3 |= 1<<PCINT25;
    // sync jack setup
    // PD3 is tip  - clock
    // PD2 is ring - start/stop
    DDRD |= (1<<DDD2) | (1<<DDD3);
    PORTD &= ~((1<<PORTD2) | (1<<PORTD3));
    lcd_init();
    OPL2_init();
    GPIO_init();
    
    // 1/64 prescale
    TCCR3B |= (1<<CS30);
    TCCR3B |= (1<<CS31);
    TCCR3B &= ~(1<<CS32);
    // set timer to fast pwm mode
    TCCR3A |= (1<<WGM30);
    TCCR3A |= (1<<WGM31);
    TCCR3B |= (1<<WGM32);
    TCCR3B |= (1<<WGM33);
    
    OCR3A = F_CPU / ((float)(64 * tempo * 4) / 60.0);
    OCR3B = OCR3A / 2.0;
    
    TIMSK3 |= (1<<OCIE3A) | (1<<OCIE3B);
    sei();
}

void nextStep(void) {
    // play all the notes for this step
    for (int i = 0; i < NUM_INST; i++) {
        if (steps[i][step]) {
            playNote(i, octaves[i], NOTE_C);
        }
    }
    GPIO_setLED(step == 0 ? 15 : step - 1, 0 | steps[current_channel][step - 1], 0);
    GPIO_setLED(step, 1, 1);
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

void handleStepInput(void) {
    uint16_t step_buttons = GPIO_readSteps();
    for (int i = 0; i < 16; i++) {
        if ((step_buttons>>i) & 0x1) {
            steps[current_channel][i] = !steps[current_channel][i];
            GPIO_setLED(i, steps[current_channel][i], 0);
            break;
        }
    }
    input_intrpt1 = 0;
}

void switchChannel(int new_channel) {
    // add drawblocks for the settings bars
    LCD_drawBlock(5 + current_channel * 12, 5, 12 + current_channel * 12, 12, WHITE);
    LCD_drawBlock(5 + new_channel * 12, 5, 12 + new_channel * 12, 12, BLACK);
    GPIO_setAllLEDsArray(steps[new_channel]);
    current_channel = new_channel;
}

void pressStart(void) {
    if (stopped) {
        send_start = 1;
    }
    stopped = !stopped;
}

void handleButtonInput(void) {
    uint16_t buttons = GPIO_readButtons();
    switch (buttons & B_MASK) {
        case B_BD:
            switchChannel(BD);
            break;
        case B_SN:
            switchChannel(SN);
            break;
        case B_CL:
            switchChannel(CL);
            break;
        case B_CY:
            switchChannel(CY);
            break;
        case B_OH:
            switchChannel(OH);
            break;
        case B_CH:
            switchChannel(CH);
            break;
        case B_STRT:
            pressStart();
            break;
    }
    input_intrpt2 = 0;
}

void setUpScreen(void) {
    LCD_setScreen(BLACK);
    LCD_drawString(80, 5, "Drum Machine", WHITE, BLACK);
    for (int i = 0; i < NUM_INST; i++) {
        LCD_drawBlock(4 + i * 12, 4, 13 + i * 12, 13, WHITE);
    }
    char* labels[] = { " A ", " B ", "TUN", "VOL" };
    for (int i = 0; i < 4; i++) {
        LCD_drawBlock(22 + i * 32, 36, 42 + i * 32, 96, WHITE);
        LCD_drawBlock(24 + i * 32, 38, 40 + i * 32, 94, BLACK);
        LCD_drawString(24 + i * 32, 100, labels[i], WHITE, BLACK);
        // delete next line
        LCD_drawBlock(23 + i * 32, 40 + i * 10, 40 + i * 32, 94, WHITE);
    }
    const uint8_t qNote[] = {
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00111100,
        0b01111100,
        0b01111100,
        0b00111000,
    };
    int x = 112, y = 18;
    for(int i = 0; i < 10; i++){
        uint8_t pixels = qNote[i]; //Go through the list of pixels
        for(int j = 0; j < 8; j++){
            if (((pixels<<j) & 0x80)){
                LCD_drawPixel(x+j,y+i,WHITE);
            }
        }
   }
   LCD_drawChar(x + 9, y + 2, '=', WHITE, BLACK);
   sprintf(tempoStr, "%d", tempo);
   LCD_drawString(x + 17, y + 2, tempoStr, WHITE, BLACK);
}

void toggleSelectMode() {
    if (current_setting == TEMPO) {
        LCD_drawBlock(151, 21, 156, 26, select_mode ? BLACK : WHITE);
    } else {
        LCD_drawBlock(23 + current_setting * 32, 113, 41 + current_setting * 32, 115, select_mode ? BLACK : WHITE);
    } 
    select_mode = !select_mode;
}

void modifySetting(int change) {
    uint8_t* setting;
    if (current_setting == TEMPO) {
        tempo = CLAMP(tempo + change, 60, 300);
    } else {
        setting = &(settings[current_channel][current_setting]);
        *setting = CLAMP(*setting + change, 0, 255);
    }    
    switch(current_setting) {
        case PAR_A:
            break;
        case PAR_B:
            break;
        case TUN:
            break;
        case VOL:
            setChannelVolume(current_setting, *setting);
            break;
        case TEMPO:
            sprintf(tempoStr, "%d", tempo);
            LCD_drawString(129, 20, tempoStr, WHITE, BLACK);
            OCR3A = F_CPU / ((float)(64 * tempo * 4) / 60.0);
            OCR3B = OCR3A / 2.0;
            break;
    }
}

void selectSetting(int new_setting) {
    for (int i = 0; i < 4; i++) {
        LCD_drawBlock(22 + i * 32, 112, 42 + i * 32, 116, BLACK);
    }
    LCD_drawBlock(150, 20, 157, 27, BLACK);
    if (new_setting == TEMPO) {
        LCD_drawBlock(150, 20, 157, 27, WHITE);
    } else {
        LCD_drawBlock(22 + new_setting * 32, 112, 42 + new_setting * 32, 116, WHITE);
    }
    current_setting = new_setting;
}

void handleEncoderInput(void) {
    uint8_t encoder_input = GPIO_readEncoder();
    switch(encoder_input & ENC_MASK) {
        case ENC_UP:
            if (select_mode) {
                selectSetting(CLAMP(current_setting + 1, 0, 4));
            } else {
                modifySetting(1);
            }
            break;
        case ENC_DOWN:
            if (select_mode) {
                selectSetting(CLAMP(current_setting - 1, 0, 4));
            } else {
                modifySetting(-1);
            }
            break;
        case ENC_PRESS:
            toggleSelectMode();
            break;
    }
    encoder_intrpt = 0;
}

int main(void) {
    init();
    setUpInstruments();
    setUpScreen();
    _delay_ms(200);
    switchChannel(BD);
    selectSetting(PAR_A);
    while (1) {
        if (next_step) {
            nextStep();
            continue;
        }
        if (input_intrpt1) {
            handleStepInput();
            continue;
        } 
        if (input_intrpt2) {
            handleButtonInput();
            continue;
        }
        if (encoder_intrpt) {
            handleEncoderInput();
            continue;
        }
    }
}