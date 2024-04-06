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

#define GPIO_ADDR1 0x58  // GPIO expander for LEDs
#define GPIO_ADDR2 0x59  // GPIO expander for step buttons
#define GPIO_ADDR3 0x5A  // GPIO expander for any other buttons

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

static uint16_t start(void) {
    TWCR0 =  (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
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

uint16_t receive(uint8_t slave_addr, uint8_t* data, int num_bytes) {
    uint16_t error;
    
    error = start();
    if (error) {
        return error;
    }
    
    error = send_addr((slave_addr << 1) | TW_READ);
    if (error) {
        return error;
    }
    
    for (int i = 0; i < num_bytes-1; ++i) {
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
    TWBR0 = 24;
    
    // gpio 1 starts in correct state (all outputs and low, no interrupt), 2 and 3 must be set up
    // set all pins as inputs
    // set as inputs
    transmit(GPIO_ADDR2, GPIO_REG_CONFIG0, b(1){0xFF}, 1);
    transmit(GPIO_ADDR2, GPIO_REG_CONFIG1, b(1){0xFF}, 1);
    transmit(GPIO_ADDR3, GPIO_REG_CONFIG0, b(1){0xFF}, 1);
    transmit(GPIO_ADDR3, GPIO_REG_CONFIG1, b(1){0xFF}, 1);
    // enable interrupts
    transmit(GPIO_ADDR2, GPIO_REG_INTENABLE0, b(1){0x1}, 1);
    transmit(GPIO_ADDR2, GPIO_REG_INTENABLE1, b(1){0x1}, 1);
    transmit(GPIO_ADDR3, GPIO_REG_INTENABLE0, b(1){0x1}, 1);
    transmit(GPIO_ADDR3, GPIO_REG_INTENABLE1, b(1){0x1}, 1);
}

void GPIO_setLEDs(uint16_t state) {
    
}

static uint16_t readInput(uint8_t addr) {
    return 0;
}

uint16_t GPIO_readButtons(void) {
    return readInput(GPIO_ADDR2);
}

uint16_t GPIO_readSteps(void) {
    return readInput(GPIO_ADDR3);
}