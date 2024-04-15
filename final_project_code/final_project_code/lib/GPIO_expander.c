/*
 * GPIO_expander.c
 *
 * Created: 3/22/2024 3:48:34 PM
 *  Author: miada
 */

#define TWSR TWSR0 // needed because util/twi.h hasnt been updated for the 328pb

#include "GPIO_expander.h"
#include <util/twi.h>

#define b(len) (uint8_t[len])

#define GPIO_ADDR1 0x59  // GPIO expander for LEDs
#define GPIO_ADDR2 0x5A  // GPIO expander for step buttons
#define GPIO_ADDR3 0x58  // GPIO expander for any other buttons
#define GPIO_ADDRENC 0x36 // rotary encoder

#define GPIO_REG_CHIPID 0x10     ///< Register for hardcode chip ID
#define GPIO_REG_SOFTRESET 0x7F  ///< Register for soft resetting
#define GPIO_REG_INPUT0 0x00     ///< Register for reading P0 input values
#define GPIO_REG_INPUT1 0x01     ///< Register for reading P1 input values
#define GPIO_REG_OUTPUT0 0x02    ///< Register for writing P0 output values
#define GPIO_REG_OUTPUT1 0x03    ///< Register for writing P1 output values
#define GPIO_REG_CONFIG0 0x04    ///< Register for configuring P0 direction
#define GPIO_REG_CONFIG1 0x05    ///< Register for configuring P1 direction
#define GPIO_REG_INTENABLE0 0x06 ///< Register for enabling P0 interrupt
#define GPIO_REG_INTENABLE1 0x07 ///< Register for enabling P1 interrupt
#define GPIO_REG_GCR 0x11        ///< Register for general configuration
#define GPIO_REG_LEDMODE0 0x12   ///< Register for configuring P0 const current
#define GPIO_REG_LEDMODE1 0x13   ///< Register for configuring P1 const current
#define GPIO_REG_LED_DIM0 0x20

#define LED_BRIGHTNESS 0x0F

static uint16_t start(void) {
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
    while (!(TWCR0 & (1<<TWINT)));
    if (TW_STATUS != TW_START) {
        return TW_STATUS;
    }
    return 0;
}   

static void stop(void) {
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

static uint16_t send_addr(uint8_t slave_addr) {
    TWDR0 = slave_addr;
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    
    while (!(TWCR0 & (1<<TWINT)));
    if (TW_STATUS != TW_MT_SLA_ACK && TW_STATUS != TW_MR_SLA_ACK) {
        return TW_STATUS;
    }
    return 0;
}

static uint16_t send_byte(uint8_t data) {
    TWDR0 = data;
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    
    while (!(TWCR0 & (1<<TWINT)));
    if (TW_STATUS != TW_MT_DATA_ACK) {
        return TW_STATUS;
    }
    return 0;
}

static uint8_t receive_byte(int read_ack) {
    if (read_ack) {
        TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
        while (!(TWCR0 & (1<<TWINT)));
        if (TW_STATUS != TW_MR_DATA_ACK) {
            return TW_STATUS;
        }
    } else {
        TWCR0 = (1<<TWINT) | (1<<TWEN);
        while (!(TWCR0 & (1<<TWINT)));
        if (TW_STATUS != TW_MR_DATA_NACK) {
            return TW_STATUS;
        }
    }
    return TWDR0;
}

static uint16_t transmit(uint8_t slave_addr, uint8_t reg, uint8_t* data, int num_bytes) {
    uint16_t error;
    
    error = start();
    if (error) {
        return error;
    }
    
    error = send_addr((slave_addr << 1) | TW_WRITE);
    if (error) {
        return error;
    }
    
    error = send_byte(reg);
    if (error) {
        return error;
    }

    for (int i = 0; i < num_bytes; i++) {
        error = send_byte(data[i]);
        if (error) {
            return error;
        }
    }
    
    stop();

    return 0;
}

static uint16_t receive(uint8_t slave_addr, uint8_t* data, int num_bytes) {
    uint16_t error;
    
    error = start();
    if (error) {
        return error;
    }
    
    error = send_addr((slave_addr << 1) | TW_READ);
    if (error) {
        return error;
    }
    
    for (int i = 0; i < num_bytes-1; i++) {
        data[i] = receive_byte(1);
    }
    data[num_bytes-1] = receive_byte(0);

    stop();

    return 0;
}

void GPIO_init(void) {
    DDRC |= (1<<PORTC4) | (1<<PORTC5);
    PORTC |= (1<<PORTC4) | (1<<PORTC5);
    DDRC  &= ~((1<<PORTC4) | (1<<PORTC5));
    // 16M / (16 + 2*24) = 250k
    // change to 12 for 400k
    TWBR0 = 12;
    
    #define USE_GPIO3 1
    
    transmit(GPIO_ADDR1, 0x7F, b(1){0x0},1);
    transmit(GPIO_ADDR2, 0x7F, b(1){0x0},1);
    #if USE_GPIO3
    transmit(GPIO_ADDR3, 0x7F, b(1){0x0},1);
    #endif
    
    // set up gpio 1
    // set as outputs
    transmit(GPIO_ADDR1, GPIO_REG_CONFIG0, b(2){0x0, 0x0}, 2);
    // set p1 to push-pull mode
    transmit(GPIO_ADDR1, GPIO_REG_GCR, b(1){0b00010000}, 1);
    // set p1 and p0 to led drive mode
    transmit(GPIO_ADDR1, GPIO_REG_LEDMODE0, b(2){0x0, 0x0}, 2);
    transmit(GPIO_ADDR1, GPIO_REG_LED_DIM0, b(16){
        0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0}, 16);
    // set up gpio 2 and 3
    // set push-pull mode
    transmit(GPIO_ADDR2, GPIO_REG_GCR, b(1){0b00010000}, 1);
    #if USE_GPIO3
    transmit(GPIO_ADDR3, GPIO_REG_GCR, b(1){0b00010000}, 1);
    #endif
    // set as inputs
    transmit(GPIO_ADDR2, GPIO_REG_CONFIG0, b(2){0xFF, 0x0}, 2);
    #if USE_GPIO3
    transmit(GPIO_ADDR3, GPIO_REG_CONFIG0, b(2){0xFF, 0x0}, 2);
    #endif
    // enable interrupts
    transmit(GPIO_ADDR2, GPIO_REG_INTENABLE0, b(2){0x00, 0xFF}, 2);
    #if USE_GPIO3
    transmit(GPIO_ADDR3, GPIO_REG_INTENABLE0, b(2){0xFC, 0xFF}, 2);
    #endif
    // clear the interrupt
    GPIO_readSteps();
    GPIO_readButtons();
    // turn off all leds
    GPIO_setAllLEDs(0x0);
}

void GPIO_setAllLEDs(uint16_t state) {
    uint8_t bytes[16];
    for (int i = 0; i < 16; i++) {
        bytes[i] = (state >> i) & 0x1 ? LED_BRIGHTNESS : 0;
    }
    transmit(GPIO_ADDR1, GPIO_REG_LED_DIM0, bytes, 16);
}

void GPIO_setAllLEDsArray(uint8_t* state) {
    uint8_t bytes[16];
    for (int i = 15; i >= 0; i--) {
        bytes[i] = state[i] ? LED_BRIGHTNESS : 0;
    }
    transmit(GPIO_ADDR1, GPIO_REG_LED_DIM0, bytes, 16);
}

void GPIO_setLED(uint8_t LED, uint8_t onOff, uint8_t bright) {
    transmit(GPIO_ADDR1, GPIO_REG_LED_DIM0 + LED, b(1){onOff ? (bright ? 0x5F : LED_BRIGHTNESS) : 0x0}, 1);
}

static uint16_t readInput(uint8_t addr) {
    uint8_t data[2] = {0x0, 0x0};
    transmit(addr, GPIO_REG_INPUT0, (void*)0, 0);
    receive(addr, data, 1);
    return ((uint16_t)data[1] << 8) | data[0];
}

uint16_t GPIO_readButtons(void) {
    return readInput(GPIO_ADDR3);
}

uint16_t GPIO_readSteps(void) {
    return readInput(GPIO_ADDR2);
}

uint8_t GPIO_readEncoder(void) {
    return 0;
}