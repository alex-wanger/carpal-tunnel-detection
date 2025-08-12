#include <avr/interrupt.h>
#include <avr/io.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/twi.h>
#include "twi.h"

static inline void clear_twsto(void) {
    while (TWCR & (1 << TWSTO)) { }
}

static inline void return_isr(twi_status_t s) {
    twi_isr.status = s;
    twi_isr.idle   = true;
}

static bool calculate_clock(uint32_t scl_hz, uint8_t *twsr_presc_code, uint8_t *twbr_val) {
    static const uint16_t presc[4] = {1, 4, 16, 64};
    for (uint8_t code = 0; code < 4; ++code) {
        long base = ((long)F_CPU / (long)scl_hz) - 16;
        if (base <= 0) continue;
        long twbr = base / (2L * presc[code]);
        if (twbr > 0 && twbr < 256) {
            *twsr_presc_code = code;
            *twbr_val        = (uint8_t)twbr;
            return true;
        }
    }
    return false;
}

volatile twi_isr_t twi_isr = {
    .idle = true,
    .messages = NULL,
    .message_count = 0,
    .status = SUCCESS
};

twi_status_t twi_init(const uint32_t scl_frequency)
{
    DDRD &= (uint8_t)~((1 << PD4) | (1 << PD5));
    PORTD |= (1 << PD4) | (1 << PD5);
    uint8_t presc_code = 0, twbr = 0;
    if (!calculate_clock(scl_frequency, &presc_code, &twbr)) {
        return INIT_FAILURE;
    }
    TWSR &= (uint8_t)~((1 << TWPS1) | (1 << TWPS0));
    TWSR |= presc_code & 0x03;
    TWBR = twbr;
    twi_isr.idle   = true;
    twi_isr.status = SUCCESS;
    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
    return SUCCESS;
}

twi_status_t twi_enqueue(twi_message_t *messages, size_t count)
{
    uint8_t sreg = SREG;
    cli();
    if (!(TWCR & (1 << TWEN))) {
        SREG = sreg;
        return DISABLED;
    }
    if (!twi_isr.idle) {
        SREG = sreg;
        return BUSY;
    }
    twi_isr.idle          = false;
    twi_isr.messages      = messages;
    twi_isr.message_count = count;
    twi_isr.status        = SUCCESS;
    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA);
    SREG = sreg;
    return SUCCESS;
}

twi_status_t twi_status(void)
{
    uint8_t sreg = SREG;
    cli();
    twi_status_t s = twi_isr.status;
    SREG = sreg;
    return s;
}

ISR(TWI_vect)
{
    static uint8_t *buffer;
    static size_t   size;
    twi_status_t status = FAILURE;
    if (TWCR & (1 << TWWC)) {
        status = OVERWRITE_FAILURE;
        goto error;
    }
    twi_message_t *const message = (twi_message_t *)twi_isr.messages;
    switch (TW_STATUS)
    {
        case TW_START:
        case TW_REP_START:
        {
            TWDR = message->address;
            buffer = message->buffer;
            size   = message->size;
            uint8_t twcr = (TWCR & ~(1 << TWSTA));
            const bool mr_mode = (message->address & 0x01u) != 0;
            if (mr_mode) {
                twcr |= (1 << TWEA);
            }
            twcr |= (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            TWCR  = twcr;
            return;
        }
        case TW_MT_SLA_ACK:
        case TW_MT_DATA_ACK:
        {
            if (size == 0) {
                twi_isr.message_count--;
                if (twi_isr.message_count > 0) {
                    twi_isr.messages++;
                    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA);
                } else {
                    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO);
                    clear_twsto();
                    return_isr(SUCCESS);
                }
                return;
            }
            TWDR = *buffer++;
            size--;
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            return;
        }
        case TW_MR_SLA_ACK:
        {
            if (size > 1) {
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWEA);
            } else {
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            }
            return;
        }
        case TW_MR_DATA_ACK:
        {
            *buffer++ = TWDR;
            size--;
            if (size > 1) {
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWEA);
            } else if (size == 1) {
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            } else {
                twi_isr.message_count--;
                if (twi_isr.message_count > 0) {
                    twi_isr.messages++;
                    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA);
                } else {
                    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO);
                    clear_twsto();
                    return_isr(SUCCESS);
                }
            }
            return;
        }
        case TW_MR_DATA_NACK:
        {
            *buffer++ = TWDR;
            size--;
            twi_isr.message_count--;
            if (twi_isr.message_count > 0) {
                twi_isr.messages++;
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA);
            } else {
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO);
                clear_twsto();
                return_isr(SUCCESS);
            }
            return;
        }
        case TW_MT_SLA_NACK:
        case TW_MR_SLA_NACK:
        case TW_MT_DATA_NACK:
        {
            twi_isr.status = NACK_FAILURE;
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO);
            twi_isr.idle = true;
            return;
        }
        default:
        {
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
            twi_isr.idle = true;
            clear_twsto();
            return;
        }
    }
error:
    return_isr(status);
    TWCR |= (1 << TWSTO);
    TWCR |= (1 << TWINT);
    clear_twsto();
}
