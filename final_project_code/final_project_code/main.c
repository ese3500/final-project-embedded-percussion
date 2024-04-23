/*
 * final_project_code.c
 *
 * Created: 3/22/2024 1:50:24 PM
 * Author : Mia McMahill & Madison Hughes
 */ 
#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "lib/OPL2.h"
#include "lib/ST7735.h"
#include "lib/LCD_GFX.h"
#include "lib/GPIO_expander.h"

#define BD 0 // bass drum
#define SN 1 // snare
#define LT 2 // low tom
#define HT 3 // hi tom
#define CH 4 // closed hat
#define OH 5 // open hat

#define PAR_A 0
#define PAR_B 1
#define TUN   2
#define VOL   3
#define TEMPO 4

#define B_MASK 0x7F
#define B_BD   0x01
#define B_SN   0x02
#define B_LT   0x04
#define B_HT   0x08
#define B_CH   0x10
#define B_OH   0x20
#define B_STRT 0x40

#define ENC_MASK  0x07
#define ENC_UP    0x01
#define ENC_DOWN  0x02
#define ENC_PRESS 0x04

#define NUM_INST 6

#define TUN_MAX 95
#define VOL_MAX 0x3F

//uint8_t steps[6][16] = {
    //{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}, // BD
    //{0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, // SN
    //{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // LT
    //{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0}, // HT
    //{0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0}, // CH
    //{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}  // OH 
//};
uint8_t steps[6][16];
uint8_t settings[6][4];
uint8_t octaves[6] = {3, 3, 4, 4, 3, 3};
uint8_t notes[6] = {NOTE_C, NOTE_C, NOTE_C, NOTE_E, NOTE_E, NOTE_E};
uint16_t tempo = 96;
char tempoStr[5];
int step = 0;
int current_channel = BD;
int stopped = 1;
int select_mode = 1;
int current_setting = 0;
int enc_button_last = 0;
volatile int send_start = 0;
volatile int ignore_next = 0;
volatile int next_step = 0;
volatile int input_intrpt1 = 0;
volatile int input_intrpt2 = 0;

typedef void (*Setter)(byte, byte, byte);
typedef byte (*Getter)(byte, byte);
typedef struct {
    Setter setter;
    Getter getter;
    int operator;
    int max;
} Setting;
void placeholderS(byte x, byte y, byte z) {};
byte placeholderG(byte x, byte y) {return 0;};
const Setting instrumentsAB[6][2] = {
    {{&setDecay, &getDecay, 1, 0xF}, {&setDecay, &getDecay, 0, 0xF}},
    {{&placeholderS, &placeholderG, 0, 255}, {&placeholderS, &placeholderG, 0, 255}},
    {{&placeholderS, &placeholderG, 0, 255}, {&placeholderS, &placeholderG, 0, 255}},
    {{&placeholderS, &placeholderG, 0, 255}, {&placeholderS, &placeholderG, 0, 255}},
    {{&placeholderS, &placeholderG, 0, 255}, {&placeholderS, &placeholderG, 0, 255}},
    {{&placeholderS, &placeholderG, 0, 255}, {&placeholderS, &placeholderG, 0, 255}}
};

const uint8_t DRUMINS_BASS_DR[11]    PROGMEM = { 0x30, 0x01, 0x07, 0xFA, 0xFD, 0x00, 0x01, 0x00, 0xF6, 0x47, 0x05 };
const uint8_t DRUMINS_SNARE[11]     PROGMEM = { 0x30, 0x24, 0x00, 0xFF, 0x0F, 0x00, 0x02, 0x00, 0xF7, 0xA9, 0x0F };
const uint8_t DRUMINS_HIHAT_CL[11]  PROGMEM = { 0x30, 0x2C, 0x00, 0xF2, 0xFE, 0x00, 0x02, 0x06, 0xB8, 0xD8, 0x37 };
const uint8_t DRUMINS_HIHAT_OP[11]  PROGMEM = { 0x30, 0x2E, 0x00, 0x82, 0xF6, 0x00, 0x04, 0x10, 0x74, 0xF8, 0x35 };
const uint8_t DRUMINS_TOMS[11]      PROGMEM = { 0x30, 0x06, 0x0A, 0xFA, 0x1F, 0x00, 0x11, 0x00, 0xF5, 0xF5, 0x0C };

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
}

ISR(TIMER3_COMPB_vect) {
    PORTD &= ~(1<<PORTD3);
}

void initSettings(void) {
    for (int i = 0; i < NUM_INST; i++) {
        settings[i][PAR_A] = instrumentsAB[i][PAR_A].getter(i, instrumentsAB[i][PAR_A].operator);
        settings[i][PAR_B] = instrumentsAB[i][PAR_B].getter(i, instrumentsAB[i][PAR_B].operator);
        settings[i][TUN]   = TUN_MAX - (octaves[i] * 12 + notes[i]);
        settings[i][VOL]   = getChannelVolume(i);
    }
}

void init(void) {
    cli();
    CLKPR = 1<<CLKPCE;
    CLKPR = 1<<CLKPS0;
    DDRE &= ~((1<<DDE0) | (1<<DDE1));
    PORTE |= (1<<PORTE0) | (1<<PORTE1);
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
    
    OCR3A = F_CPU / ((float)(64 * (uint32_t)tempo * 4) / 60.0);
    OCR3B = OCR3A / 2.0;
    
    TIMSK3 |= (1<<OCIE3A) | (1<<OCIE3B);
    
    // 1/2024 prescale
    TCCR1B |= (1<<CS10);
    TCCR1B &= ~(1<<CS11);
    TCCR1B |= (1<<CS12);
    // set timer to fast pwm mode
    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B &= ~(1<<WGM12);
    TCCR1B &= ~(1<<WGM13);
    sei();
}

void nextStep(void) {
    // play all the notes for this step
    PORTD |= (1<<PORTD2);
    PORTD |= (1<<PORTD3);
    for (int i = 0; i < NUM_INST; i++) {
        if (steps[i][step]) {
            playNote(i, octaves[i], notes[i]);
            if (i == CH) {
                setKeyOn(OH, 0);
                break;
            }
        }
    }
    GPIO_setLED(step == 0 ? 15 : step - 1, step == 0 ? steps[current_channel][15] : steps[current_channel][step - 1], 0);
    GPIO_setLED(step, 1, 1);
    next_step = 0;
    step = (step + 1) % 16;
}

void setUpInstruments(void) {
    Instrument bass_drum = loadInstrument(DRUMINS_BASS_DR, 1);
    setInstrument(BD, bass_drum, 1.0);
    
    Instrument snare_drum = loadInstrument(DRUMINS_SNARE, 1);
    setInstrument(SN, snare_drum, 1.0);
    
    Instrument ltom = loadInstrument(DRUMINS_TOMS, 1);
    setInstrument(LT, ltom, 1.0);
    
    Instrument htom = loadInstrument(DRUMINS_TOMS, 1);
    setInstrument(HT, htom, 1.0);
    
    Instrument closed_hat = loadInstrument(DRUMINS_HIHAT_CL, 1);
    setInstrument(CH, closed_hat, 1.0);
    
    Instrument open_hat = loadInstrument(DRUMINS_HIHAT_OP, 1);
    setInstrument(OH, open_hat, 1.0);
}

void handleStepInput(void) {
    input_intrpt1 = 0;
    uint16_t step_buttons = GPIO_readSteps();
    for (int i = 0; i < 16; i++) {
        if ((step_buttons>>i) & 0x1) {
            steps[current_channel][i] = !steps[current_channel][i];
            GPIO_setLED(i, steps[current_channel][i], 0);
            break;
        }
    }
}

void redrawSetting(int setting, int max) {
    LCD_drawBlock(24 + setting * 32, 38, 40 + setting * 32, 94, BLACK);
    int16_t y = 38 + (56.0 / max) * settings[current_channel][setting];
    LCD_drawBlock(24 + setting * 32, y, 40 + setting * 32, 94, WHITE);
}

void switchChannel(int new_channel) {
    // add drawblocks for the settings bars
    LCD_drawBlock(5 + current_channel * 12, 5, 12 + current_channel * 12, 12, WHITE);
    LCD_drawBlock(5 + new_channel * 12, 5, 12 + new_channel * 12, 12, BLACK);
    redrawSetting(PAR_A, instrumentsAB[current_channel][PAR_A].max);
    redrawSetting(PAR_B, instrumentsAB[current_channel][PAR_B].max);
    redrawSetting(TUN, TUN_MAX);
    redrawSetting(VOL, VOL_MAX);
    GPIO_setAllLEDsArray(steps[new_channel]);
    current_channel = new_channel;
}

void pressStart(void) {
    _delay_ms(20);
    if (stopped) {
        step = 0;
        send_start = 1;
        PORTD &= ~(1<<PORTD3);
    } else {
        PORTD &= ~(1<<PORTD2);
        PORTD &= ~(1<<PORTD3);
    }
    stopped = !stopped;
    step = 0;
    GPIO_setAllLEDsArray(steps[current_channel]);
}

void handleButtonInput(void) {
    input_intrpt2 = 0;
    uint16_t buttons = GPIO_readButtons();
    switch (buttons & B_MASK) {
        case B_BD:
            switchChannel(BD);
            break;
        case B_SN:
            switchChannel(SN);
            break;
        case B_LT:
            switchChannel(LT);
            break;
        case B_HT:
            switchChannel(HT);
            break;
        case B_CH:
            switchChannel(CH);
            break;
        case B_OH:
            switchChannel(OH);
            break;
        case B_STRT:
            pressStart();
            break;
    }
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
    uint8_t* setting = NULL;
    int max, min = 0;
    switch (current_setting) {
        case PAR_A:
        case PAR_B:
            max = instrumentsAB[current_channel][current_setting].max;
            break;
        case TUN:
            max = TUN_MAX;
            break;
        case VOL:
            max = VOL_MAX;
            break;
        case TEMPO:
            min = 60;
            max = 300;
            break;
        default:
            return;
    }
    switch(current_setting) {
        case PAR_A:
        case PAR_B:
            setting = &(settings[current_channel][current_setting]);
            *setting = CLAMP(*setting + change, min, max);
            instrumentsAB[current_channel][current_setting].setter(current_channel, instrumentsAB[current_channel][current_setting].operator, *setting);
            break;
        case TUN:
            setting = &(settings[current_channel][current_setting]);
            *setting = CLAMP(*setting + change, min, max);
            octaves[current_channel] = (TUN_MAX - *setting) / 12;
            notes[current_channel]   = (TUN_MAX - *setting) % 12;
            break;
        case VOL:
            setting = &(settings[current_channel][current_setting]);
            *setting = CLAMP(*setting + change, min, max);
            setChannelVolume(current_channel, *setting);
            break;
        case TEMPO:
            tempo = CLAMP(tempo - change, 60, 300);
            sprintf(tempoStr, tempo >= 100 ? "%d" : "%d " , tempo);
            LCD_drawString(129, 20, tempoStr, WHITE, BLACK);
            OCR3A = F_CPU / ((float)(64 * (uint32_t)tempo * 4) / 60.0);
            OCR3B = OCR3A / 2.0;
            return;
    }
    redrawSetting(current_setting, max);
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
    cli();
    uint8_t encoder_input = GPIO_readEncoder();
    switch(encoder_input & ENC_MASK) {
        case ENC_UP:
            if (select_mode) {
                selectSetting(CLAMP(current_setting + 1, 0, 4));
            } else {
                modifySetting(-1);
            }
            break;
        case ENC_DOWN:
            if (select_mode) {
                selectSetting(CLAMP(current_setting - 1, 0, 4));
            } else {
                modifySetting(1);
            }
            break;
        case ENC_PRESS:
            if (!enc_button_last) {
                toggleSelectMode();
            }
            break;
    }
    enc_button_last = encoder_input & ENC_PRESS;
    sei();
}

void clearFlags(void) {
    input_intrpt1 = 0;
    input_intrpt2 = 0;
}

int main(void) {
    _delay_ms(400);
    init();
    setUpInstruments();
    initSettings();
    setUpScreen();
    _delay_ms(200);
    switchChannel(BD);
    selectSetting(PAR_A);
    clearFlags();
    while (1) {
        if (next_step) {
            nextStep();
            continue;
        }
        if (input_intrpt1 && !next_step) {
            handleStepInput();
            continue;
        }
        if (input_intrpt2 && !next_step) {
            handleButtonInput();
            continue;
        }
        if (TCNT1 % 512 == 0 && !next_step) {
            handleEncoderInput();
            continue;
        }
    }
}