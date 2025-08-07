#include "twi.h"
#include <avr/io.h>
#include <util/twi.h>

void twi_init(void) {
    TWSR = 0x00;
    TWBR = ((F_CPU / 100000UL) - 16) / 2;  // SCL = 100kHz
}

void twi_start(void) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

void twi_stop(void) {
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

uint8_t twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return (TWSR & 0xF8);
}

void twi_writeReg(uint8_t slave_addr, uint8_t reg, uint8_t data) {
    twi_start();
    twi_write((slave_addr<<1)|0);
    twi_write(reg);
    twi_write(data);
    twi_stop();
}

uint8_t twi_readAck(void) {
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t twi_readNack(void) {
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

void twi_readRegs(uint8_t slave_addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    twi_start();
    twi_write((slave_addr<<1)|0);
    twi_write(reg);
    twi_start();
    twi_write((slave_addr<<1)|1);
    for (uint8_t i = 0; i < len-1; i++) {
        buf[i] = twi_readAck();
    }
    buf[len-1] = twi_readNack();
    twi_stop();
}

