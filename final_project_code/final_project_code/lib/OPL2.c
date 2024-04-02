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

#define TRUE  1
#define FALSE 0

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

void SPI_transfer(byte value) {
    SPDR1 = value;
    asm volatile("nop");
    while(!(SPSR1 & (1<<SPIF1)));
}

void write(byte reg, byte value) {
    // Write OPL2 address.
    PORTD &= ~A0;
    SPI_transfer(reg);
    PORTE &= ~LATCH;
    _delay_us(16);
    PORTE |= LATCH;
    _delay_us(16);

    // Write OPL2 data.
    PORTD |= A0;
    SPI_transfer(value);
    PORTE &= ~LATCH;
    _delay_us(4);
    PORTE |= LATCH;
    _delay_us(92);
}

byte getRegisterOffset(byte channel, byte operatorNum) {
    return registerOffsets[operatorNum % 2][channel % NUM_CHANNELS];
}

byte getChipRegisterOffset(int16_t reg) {
    switch (reg & 0xFF) {
        case 0x08:
            return 1;
        case 0xBD:
            return 2;
        case 0x01:
        default:
            return 0;
    }
}

byte getChipRegister(short reg) {
    return chipRegisters[getChipRegisterOffset(reg)];
}

void setChipRegister(int16_t reg, byte value) {
    chipRegisters[getChipRegisterOffset(reg)] = value;
    write(reg & 0xFF, value);
}

byte getChannelRegisterOffset(byte baseRegister, byte channel) {
    channel = channel % NUM_CHANNELS;
    byte offset = channel * 3;

    switch (baseRegister) {
        case 0xB0:
            return offset + 1;
        case 0xC0:
            return offset + 2;
        case 0xA0:
        default:
            return offset;
    }
}

byte getChannelRegister(byte baseRegister, byte channel) {
    return channelRegisters[getChannelRegisterOffset(baseRegister, channel)];
}

void setChannelRegister(byte baseRegister, byte channel, byte value) {
    channelRegisters[getChannelRegisterOffset(baseRegister, channel)] = value;
    byte reg = baseRegister + (channel % NUM_CHANNELS);
    write(reg, value);
}

int16_t getOperatorRegisterOffset(byte baseRegister, byte channel, byte operatorNum) {
    channel = channel % NUM_CHANNELS;
    operatorNum = operatorNum & 0x01;
    int16_t offset = (channel * 10) + (operatorNum * 5);

    switch (baseRegister) {
        case 0x40:
            return offset + 1;
        case 0x60:
            return offset + 2;
        case 0x80:
            return offset + 3;
        case 0xE0:
            return offset + 4;
        case 0x20:
        default:
            return offset;
    }
}

byte getOperatorRegister(byte baseRegister, byte channel, byte operatorNum) {
    return operatorRegisters[getOperatorRegisterOffset(baseRegister, channel, operatorNum)];
}

void setOperatorRegister(byte baseRegister, byte channel, byte operatorNum, byte value) {
    operatorRegisters[getOperatorRegisterOffset(baseRegister, channel, operatorNum)] = value;
    byte reg = baseRegister + getRegisterOffset(channel, operatorNum);
    write(reg, value);
}

void OPL2_reset(void) {
    // Hard reset the OPL2.
    PORTD &= ~RESET;
    _delay_ms(1);
    PORTD |= RESET;

   	// Initialize chip registers.
   	setChipRegister(0x00, 0x00);
   	setChipRegister(0x08, 0x40);
   	setChipRegister(0xBD, 0x00);

   	// Initialize all channel and operator registers.
   	for (byte i = 0; i < NUM_CHANNELS; i ++) {
       	setChannelRegister(0xA0, i, 0x00);
       	setChannelRegister(0xB0, i, 0x00);
       	setChannelRegister(0xC0, i, 0x00);

       	for (byte j = OPERATOR1; j <= OPERATOR2; j ++) {
           	setOperatorRegister(0x20, i, j, 0x00);
           	setOperatorRegister(0x40, i, j, 0x3F);
           	setOperatorRegister(0x60, i, j, 0x00);
           	setOperatorRegister(0x80, i, j, 0x00);
           	setOperatorRegister(0xE0, i, j, 0x00);
       	}
   	}
}

void createShadowRegisters(void) {
    for (int i = 0; i < 3; i++) {
        chipRegisters[i] = 0;
    }
    for (int i = 0; i < 27; i++) {
        channelRegisters[i] = 0;
    }
    for (int i = 0; i < 90; i++) {
        operatorRegisters[i] = 0;
    }
}

void OPL2_init(void) {
    // set up SPI1
    DDRC  |= SHIFT;
    DDRE  |= DATA | LATCH;
    PORTC &= ~SHIFT;
    PORTE |= LATCH;
    PORTE &= ~DATA;
    SPCR1 |= (1<<SPE1) | (1<<MSTR1);
    SPSR1 &= ~(1<<SPI2X1);
    //SPCR1 |= (1<<SPE1) | (1<<MSTR1) | (((1 ^ 0x1) >> 1) & 0x03) | (0x00 & 0x0C);
    //SPSR1 |= (1 ^ 0x1) & 0x1;
    //SPSR1 |= (1<<SPI2X1);
    
    // set up other pins
    DDRD |= A0;
    DDRD |= RESET;

    PORTD &= ~A0;
    PORTD |= RESET;
    
    createShadowRegisters();
    OPL2_reset();
}

byte getBlock(byte channel) {
    return (getChannelRegister(0xB0, channel) & 0x1C) >> 2;
}

void setBlock(byte channel, byte block) {
    byte value = getChannelRegister(0xB0, channel) & 0xE3;
    setChannelRegister(0xB0, channel, value + ((block & 0x07) << 2));
}

float getFrequencyStep(byte channel) {
    return fIntervals[getBlock(channel)];
}

int16_t getFrequencyFNumber(byte channel, int16_t frequency) {
    float fInterval = getFrequencyStep(channel);
    return CLAMP((int16_t)(frequency / fInterval), (int16_t)0, (int16_t)1023);
}

int16_t getNoteFNumber(byte note) {
    return noteFNumbers[note % NUM_NOTES];
}

byte getFrequencyBlock(float frequency) {
    for (byte i = 0; i < 8; i ++) {
        if (frequency < blockFrequencies[i]) {
            return i;
        }
    }
    return 7;
}

int getWaveFormSelect() {
    return getChipRegister(0x01) & 0x20;
}

void setWaveFormSelect(int enable) {
    if (enable) {
        setChipRegister(0x01, getChipRegister(0x01) | 0x20);
    } else {
        setChipRegister(0x01, getChipRegister(0x01) & 0xDF);
    }
}

int getTremolo(byte channel, byte operatorNum) {
    return getOperatorRegister(0x20, channel, operatorNum) & 0x80;
}

void setTremolo(byte channel, byte operatorNum, int enable) {
    byte value =  getOperatorRegister(0x20, channel, operatorNum) & 0x7F;
    setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x80 : 0x00));
}

int getVibrato(byte channel, byte operatorNum) {
    return getOperatorRegister(0x20, channel, operatorNum) & 0x40;
}

void setVibrato(byte channel, byte operatorNum, int enable) {
    byte value = getOperatorRegister(0x20, channel, operatorNum) & 0xBF;
    setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x40 : 0x00));
}

int getMaintainSustain(byte channel, byte operatorNum) {
    return getOperatorRegister(0x20, channel, operatorNum) & 0x20;
}

void setMaintainSustain(byte channel, byte operatorNum, int enable) {
    byte value = getOperatorRegister(0x20, channel, operatorNum) & 0xDF;
    setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x20 : 0x00));
}

int getEnvelopeScaling(byte channel, byte operatorNum) {
    return getOperatorRegister(0x20, channel, operatorNum) & 0x10;
}

void setEnvelopeScaling(byte channel, byte operatorNum, int enable) {
    byte value = getOperatorRegister(0x20, channel, operatorNum) & 0xEF;
    setOperatorRegister(0x20, channel, operatorNum, value + (enable ? 0x10 : 0x00));
}

byte getMultiplier(byte channel, byte operatorNum) {
    return getOperatorRegister(0x20, channel, operatorNum) & 0x0F;
}

void setMultiplier(byte channel, byte operatorNum, byte multiplier) {
    byte value = getOperatorRegister(0x20, channel, operatorNum) & 0xF0;
    setOperatorRegister(0x20, channel, operatorNum, value + (multiplier & 0x0F));
}

byte getScalingLevel(byte channel, byte operatorNum) {
    return (getOperatorRegister(0x40, channel, operatorNum) & 0xC0) >> 6;
}

void setScalingLevel(byte channel, byte operatorNum, byte scaling) {
    byte value = getOperatorRegister(0x40, channel, operatorNum) & 0x3F;
    setOperatorRegister(0x40, channel, operatorNum, value + ((scaling & 0x03) << 6));
}

byte getVolume(byte channel, byte operatorNum) {
    return getOperatorRegister(0x40, channel, operatorNum) & 0x3F;
}

void setVolume(byte channel, byte operatorNum, byte volume) {
    byte value = getOperatorRegister(0x40, channel, operatorNum) & 0xC0;
    setOperatorRegister(0x40, channel, operatorNum, value + (volume & 0x3F));
}

byte getSynthMode(byte channel) {
    return getChannelRegister(0xC0, channel) & 0x01;
}

void setSynthMode(byte channel, byte synthMode) {
    byte value = getChannelRegister(0xC0, channel) & 0xFE;
    setChannelRegister(0xC0, channel, value + (synthMode & 0x01));
}

byte getChannelVolume(byte channel) {
    return getVolume(channel, OPERATOR2);
}

void setChannelVolume(byte channel, byte volume) {
    if (getSynthMode(channel)) {
        setVolume(channel, OPERATOR1, volume);
    }
    setVolume(channel, OPERATOR2, volume);
}

byte getAttack(byte channel, byte operatorNum) {
    return (getOperatorRegister(0x60, channel, operatorNum) & 0xF0) >> 4;
}

void setAttack(byte channel, byte operatorNum, byte attack) {
    byte value = getOperatorRegister(0x60, channel, operatorNum) & 0x0F;
    setOperatorRegister(0x60, channel, operatorNum, value + ((attack & 0x0F) << 4));
}

byte getDecay(byte channel, byte operatorNum) {
    return getOperatorRegister(0x60, channel, operatorNum) & 0x0F;
}

void setDecay(byte channel, byte operatorNum, byte decay) {
    byte value = getOperatorRegister(0x60, channel, operatorNum) & 0xF0;
    setOperatorRegister(0x60, channel, operatorNum, value + (decay & 0x0F));
}

byte getSustain(byte channel, byte operatorNum) {
    return (getOperatorRegister(0x80, channel, operatorNum) & 0xF0) >> 4;
}

void setSustain(byte channel, byte operatorNum, byte sustain) {
    byte value = getOperatorRegister(0x80, channel, operatorNum) & 0x0F;
    setOperatorRegister(0x80, channel, operatorNum, value + ((sustain & 0x0F) << 4));
}

byte getRelease(byte channel, byte operatorNum) {
    return getOperatorRegister(0x80, channel, operatorNum) & 0x0F;
}

void setRelease(byte channel, byte operatorNum, byte release) {
    byte value = getOperatorRegister(0x80, channel, operatorNum) & 0xF0;
    setOperatorRegister(0x80, channel, operatorNum, value + (release & 0x0F));
}

int16_t getFNumber(byte channel) {
    int16_t value = (getChannelRegister(0xB0, channel) & 0x03) << 8;
    value += getChannelRegister(0xA0, channel);
    return value;
}

void setFNumber(byte channel, int16_t fNumber) {
    byte value = getChannelRegister(0xB0, channel) & 0xFC;
    setChannelRegister(0xB0, channel, value + ((fNumber & 0x0300) >> 8));
    setChannelRegister(0xA0, channel, fNumber & 0xFF);
}

float getFrequency(byte channel) {
    float fInterval = getFrequencyStep(channel);
    return getFNumber(channel) * fInterval;
}

void setFrequency(byte channel, float frequency) {
    byte block = getFrequencyBlock(frequency);
    if (getBlock(channel) != block) {
        setBlock(channel, block);
    }
    uint16_t fNumber = getFrequencyFNumber(channel, frequency);
    setFNumber(channel, fNumber);
}

int getNoteSelect() {
    return getChipRegister(0x08) & 0x40;
}

void setNoteSelect(int enable) {
    setChipRegister(0x08, enable ? 0x40 : 0x00);
}

int getKeyOn(byte channel) {
    return getChannelRegister(0xB0, channel) & 0x20;
}

void setKeyOn(byte channel, int keyOn) {
    byte value = getChannelRegister(0xB0, channel) & 0xDF;
    setChannelRegister(0xB0, channel, value + (keyOn ? 0x20 : 0x00));
}

byte getFeedback(byte channel) {
    return (getChannelRegister(0xC0, channel) & 0x0E) >> 1;
}

void setFeedback(byte channel, byte feedback) {
    byte value = getChannelRegister(0xC0, channel) & 0xF1;
    setChannelRegister(0xC0, channel, value + ((feedback & 0x07) << 1));
}

int getDeepTremolo() {
    return getChipRegister(0xBD) & 0x80;
}

void setDeepTremolo(int enable) {
    byte value = getChipRegister(0xBD) & 0x7F;
    setChipRegister(0xBD, value + (enable ? 0x80 : 0x00));
}

int getDeepVibrato() {
    return getChipRegister(0xBD) & 0x40;
}

void setDeepVibrato(int enable) {
    byte value = getChipRegister(0xBD) & 0xBF;
    setChipRegister(0xBD, value + (enable ? 0x40 : 0x00));
}

int getPercussion() {
    return getChipRegister(0xBD) & 0x20;
}

void setPercussion(int enable) {
    byte value = getChipRegister(0xBD) & 0xDF;
    setChipRegister(0xBD, value + (enable ? 0x20 : 0x00));
}

byte getDrums() {
    return getChipRegister(0xBD) & 0x1F;
}

void setDrumsByte(byte drums) {
    byte value = getChipRegister(0xBD) & 0xE0;
    setChipRegister(0xBD, value);
    setChipRegister(0xBD, value + (drums & 0x1F));
}

void setDrums(int bass, int snare, int tom, int cymbal, int hihat) {
    byte drums = 0;
    drums += bass   ? DRUM_BITS_BASS   : 0x00;
    drums += snare  ? DRUM_BITS_SNARE  : 0x00;
    drums += tom    ? DRUM_BITS_TOM    : 0x00;
    drums += cymbal ? DRUM_BITS_CYMBAL : 0x00;
    drums += hihat  ? DRUM_BITS_HI_HAT : 0x00;
    setDrumsByte(drums);
}

byte getWaveForm(byte channel, byte operatorNum) {
    return getOperatorRegister(0xE0, channel, operatorNum) & 0x07;
}

void setWaveForm(byte channel, byte operatorNum, byte waveForm) {
    byte value = getOperatorRegister(0xE0, channel, operatorNum) & 0xF8;
    setOperatorRegister(0xE0, channel, operatorNum, value + (waveForm & 0x07));
}

Instrument createInstrument() {
    Instrument instrument;

    for (byte op = OPERATOR1; op <= OPERATOR2; op++) {
        instrument.operators[op].hasTremolo = FALSE;
        instrument.operators[op].hasVibrato = FALSE;
        instrument.operators[op].hasSustain = FALSE;
        instrument.operators[op].hasEnvelopeScaling = FALSE;
        instrument.operators[op].frequencyMultiplier = 0;
        instrument.operators[op].keyScaleLevel = 0;
        instrument.operators[op].outputLevel = 0;
        instrument.operators[op].attack = 0;
        instrument.operators[op].decay = 0;
        instrument.operators[op].sustain = 0;
        instrument.operators[op].release = 0;
        instrument.operators[op].waveForm = 0;
    }

    instrument.transpose = 0;
    instrument.feedback = 0;
    instrument.isAdditiveSynth = FALSE;

    return instrument;
}

Instrument getInstrument(byte channel) {
    Instrument instrument;

    for (byte op = OPERATOR1; op <= OPERATOR2; op ++) {
        instrument.operators[op].hasTremolo = getTremolo(channel, op);
        instrument.operators[op].hasVibrato = getVibrato(channel, op);
        instrument.operators[op].hasSustain = getMaintainSustain(channel, op);
        instrument.operators[op].hasEnvelopeScaling = getEnvelopeScaling(channel, op);
        instrument.operators[op].frequencyMultiplier = getMultiplier(channel, op);
        instrument.operators[op].keyScaleLevel = getScalingLevel(channel, op);
        instrument.operators[op].outputLevel = getVolume(channel, op);
        instrument.operators[op].attack = getAttack(channel, op);
        instrument.operators[op].decay = getDecay(channel, op);
        instrument.operators[op].sustain = getSustain(channel, op);
        instrument.operators[op].release = getRelease(channel, op);
        instrument.operators[op].waveForm = getWaveForm(channel, op);
    }

    instrument.transpose = 0;
    instrument.feedback = getFeedback(channel);
    instrument.isAdditiveSynth = getSynthMode(channel) == SYNTH_MODE_AM;

    return instrument;
}

void setInstrument(byte channel, Instrument instrument, float volume) {
    volume = CLAMP(volume, (float)0.0, (float)1.0);

    setWaveFormSelect(TRUE);
    for (byte op = OPERATOR1; op <= OPERATOR2; op ++) {
        byte outputLevel = 63 - (byte)((63.0 - (float)instrument.operators[op].outputLevel) * volume);

        setOperatorRegister(0x20, channel, op,
        (instrument.operators[op].hasTremolo ? 0x80 : 0x00) +
        (instrument.operators[op].hasVibrato ? 0x40 : 0x00) +
        (instrument.operators[op].hasSustain ? 0x20 : 0x00) +
        (instrument.operators[op].hasEnvelopeScaling ? 0x10 : 0x00) +
        (instrument.operators[op].frequencyMultiplier & 0x0F));
        setOperatorRegister(0x40, channel, op,
        ((instrument.operators[op].keyScaleLevel & 0x03) << 6) +
        (outputLevel & 0x3F));
        setOperatorRegister(0x60, channel, op,
        ((instrument.operators[op].attack & 0x0F) << 4) +
        (instrument.operators[op].decay & 0x0F));
        setOperatorRegister(0x80, channel, op,
        ((instrument.operators[op].sustain & 0x0F) << 4) +
        (instrument.operators[op].release & 0x0F));
        setOperatorRegister(0xE0, channel, op,
        (instrument.operators[op].waveForm & 0x07));
    }

    byte value = getChannelRegister(0xC0, channel) & 0xF0;
    setChannelRegister(0xC0, channel,
    value +
    ((instrument.feedback & 0x07) << 1) +
    (instrument.isAdditiveSynth ? 0x01 : 0x00));
}

void setDrumInstrument(Instrument instrument, byte drumType, float volume) {
    drumType = CLAMP(drumType, (byte)DRUM_BASS, (byte)DRUM_HI_HAT);
    volume = CLAMP(volume, (float)0.0, (float)1.0);
    byte channel = drumChannels[drumType];

    setWaveFormSelect(TRUE);
    for (byte op = OPERATOR1; op <= OPERATOR2; op ++) {
        byte outputLevel = 63 - (byte)((63.0 - (float)instrument.operators[op].outputLevel) * volume);
        byte registerOffset = drumRegisterOffsets[op][drumType];

        if (registerOffset != 0xFF) {
            setOperatorRegister(0x20, channel, op,
            (instrument.operators[op].hasTremolo ? 0x80 : 0x00) +
            (instrument.operators[op].hasVibrato ? 0x40 : 0x00) +
            (instrument.operators[op].hasSustain ? 0x20 : 0x00) +
            (instrument.operators[op].hasEnvelopeScaling ? 0x10 : 0x00) +
            (instrument.operators[op].frequencyMultiplier & 0x0F));
            setOperatorRegister(0x40, channel, op,
            ((instrument.operators[op].keyScaleLevel & 0x03) << 6) +
            (outputLevel & 0x3F));
            setOperatorRegister(0x60, channel, op,
            ((instrument.operators[op].attack & 0x0F) << 4) +
            (instrument.operators[op].decay & 0x0F));
            setOperatorRegister(0x80, channel, op,
            ((instrument.operators[op].sustain & 0x0F) << 4) +
            (instrument.operators[op].release & 0x0F));
            setOperatorRegister(0xE0, channel, op,
            (instrument.operators[op].waveForm & 0x07));
        }
    }

    byte value = getChannelRegister(0xC0, channel) & 0xF0;
    setChannelRegister(0xC0, channel,
    value +
    ((instrument.feedback & 0x07) << 1) +
    (instrument.isAdditiveSynth ? 0x01 : 0x00));
}

Instrument loadInstrument(const unsigned char *instrumentData, int fromProgmem) {
    Instrument instrument = createInstrument();

    byte data[11];
    for (byte i = 0; i < 11; i ++) {
        if (fromProgmem) {
            data[i] = pgm_read_byte_near(instrumentData + i);
        } else {
            data[i] = instrumentData[i];
        }
    }

    for (byte op = OPERATOR1; op <= OPERATOR2; op ++) {
        instrument.operators[op].hasTremolo = data[op * 5 + 1] & 0x80 ? TRUE : FALSE;
        instrument.operators[op].hasVibrato = data[op * 5 + 1] & 0x40 ? TRUE : FALSE;
        instrument.operators[op].hasSustain = data[op * 5 + 1] & 0x20 ? TRUE : FALSE;
        instrument.operators[op].hasEnvelopeScaling = data[op * 5 + 1] & 0x10 ? TRUE : FALSE;
        instrument.operators[op].frequencyMultiplier = (data[op * 5 + 1] & 0x0F);
        instrument.operators[op].keyScaleLevel = (data[op * 5 + 2] & 0xC0) >> 6;
        instrument.operators[op].outputLevel = data[op * 5 + 2] & 0x3F;
        instrument.operators[op].attack = (data[op * 5 + 3] & 0xF0) >> 4;
        instrument.operators[op].decay = data[op * 5 + 3] & 0x0F;
        instrument.operators[op].sustain = (data[op * 5 + 4] & 0xF0) >> 4;
        instrument.operators[op].release = data[op * 5 + 4] & 0x0F;
    }
    instrument.operators[0].waveForm = data[10] & 0x07;
    instrument.operators[1].waveForm = (data[10] & 0x70) >> 4;

    instrument.transpose = data[0];
    instrument.feedback = (data[5] & 0x0E) >> 1;
    instrument.isAdditiveSynth = data[5] & 0x01 ? TRUE : FALSE;

    return instrument;
}

void playNote(byte channel, byte octave, byte note) {
    if (getKeyOn(channel)) {
        setKeyOn(channel, FALSE);
    }
    setBlock(channel, CLAMP(octave, (byte)0, (byte)NUM_OCTAVES));
    setFNumber(channel, noteFNumbers[note % 12]);
    setKeyOn(channel, TRUE);
}

void playDrum(byte drum, byte octave, byte note) {
    drum = drum % NUM_DRUM_SOUNDS;
    byte drumState = getDrums();

    setDrumsByte(drumState & ~drumBits[drum]);
    byte drumChannel = drumChannels[drum];
    setBlock(drumChannel, CLAMP(octave, (byte)0, (byte)NUM_OCTAVES));
    setFNumber(drumChannel, noteFNumbers[note % NUM_NOTES]);
    setDrumsByte(drumState | drumBits[drum]);
}