#include <avr/interrupt.h>
#include <avr/io.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util/twi.h>
#include "../include/twi.h"

static volatile twi_isr_t twi_isr;

static bool scl_parameters(uint8_t twbr_value)
{
    if (twbr_value == 0)
    {
        return false;
    }
    return true;
}

void calculate_clock(uint64_t src, uint8_t *output_array)
{
    static const uint16_t prescaler_factors[4] = {1, 4, 16, 64};

    uint64_t base = ((F_CPU / src) - 16) / 2;

    for (uint8_t code = 0; code < 4; code++)
    {
        uint64_t twbr_val = base / prescaler_factors[code];
        if (twbr_val > 0 && twbr_val < 256)
        {
            output_array[0] = code;              // TWSR prescaler code (0..3)
            output_array[1] = (uint8_t)twbr_val; // TWBR register value
            return;
        }
    }
    twi_isr.status = INIT_FAILURE;
}

twi_status_t twi_init(const uint32_t scl_frequency)
{
    // initialize pull-up resistors (assuming SDA/SCL on PD4/PD5)
    DDRD = DDRD & 0b11001111;         // set pins 4 and 5 as inputs
    PORTD |= (1 << PD4) | (1 << PD5); // enable pull-up resistors

    uint8_t output[2];

    calculate_clock(scl_frequency, output);

    if (scl_parameters(output[1]))
    {
        TWSR |= output[0]; // Set prescaler
        TWBR = output[1];  // Set TWBR value
    }
    else
    {
        return INIT_FAILURE;
    }

    twi_isr.idle = true;
    twi_isr.status = SUCCESS;

    TWCR |= (1 << TWEN) | (1 << TWIE) | (1 << TWEA);

    return SUCCESS;
}

twi_status_t twi_enqueue(twi_message_t *messages, size_t count)
{
    uint8_t sreg = SREG;

    cli();

    if (!(TWCR & (1 << TWEN)))
    {
        SREG = sreg;
        return DISABLED;
    }

    if (!twi_isr.idle)
    {
        SREG = sreg;
        return BUSY;
    }

    twi_isr = (twi_isr_t){
        .idle = false,
        .messages = messages,
        .message_count = count,
        .status = SUCCESS};

    TWCR |= (1 << TWSTA);

    SREG = sreg;

    return SUCCESS;
}

twi_status_t twi_status(void)
{
    uint8_t sreg = SREG;

    cli();

    twi_status_t status = twi_isr.status;

    SREG = sreg;

    return status;
}

static void return_isr(const twi_status_t status)
{
    twi_isr = (twi_isr_t){
        .idle = true,
        .status = status,
    };
}

static void clear_twsto(void)
{
    // If a user enqueues a new set of messages while the STOP
    // condition is asserted on the bus, triggering a START
    // condition will result in a bus error. To avoid this, we wait
    // for TWSTO to be automatically get cleared by hardware. This
    // does however introduce an unavoidable delay of ~15 us in our
    // code (when running at 16 MHz) which is irritating, however,
    // it's a tradeoff worth making if it means users of our driver
    // don't need to deal with spurious bus errors.
    while (TWCR & (1 << TWSTO))
        continue;
}

ISR(TWI_vect)
{
    static unsigned char *buffer;
    static size_t size;

    twi_status_t status = FAILURE;

    if (TWCR & (1 << TWWC))
    {
        status = OVERWRITE_FAILURE;
        goto error;
    }

    twi_message_t *const message = twi_isr.messages;

    switch (TW_STATUS)
    {
    case TW_START:
    case TW_REP_START:
        TWDR = message->address;

        buffer = message->buffer;
        size = message->size;

        uint8_t twcr = TWCR & ~(1 << TWSTA);

        const bool mr_mode = message->address & 1;

        if (mr_mode)
            twcr |= 1 << TWEA;

        twcr |= (1 << TWINT);

        TWCR = twcr;

        return;

    case TW_MT_SLA_ACK:  // SLA+W transmitted, ACK received
    case TW_MT_DATA_ACK: // Data byte transmitted, ACK received
        if (size == 0)
        {
            twi_isr.message_count--;
            if (twi_isr.message_count > 0)
            {
                twi_isr.messages++;
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA); // repeated START
            }
            else
            {
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO); // STOP
                clear_twsto();
                return_isr(SUCCESS);
            }
            return;
        }

        // Send next data byte
        TWDR = *(buffer);
        buffer++;
        size--;
        TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // Prepare for next byte
        return;

    case TW_MR_SLA_ACK: // SLA+R transmitted, ACK received
        if (size > 1)
        {
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWEA); // ACK next byte
        }
        else
        {
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // NACK last byte
        }
        return;
    case TW_MR_DATA_ACK: // Data byte received, ACK transmitted

        *(buffer) = TWDR;
        buffer++;
        size--;

        if (size > 1)
        {
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWEA); // ACK next byte
        }

        else if (size == 1)
        {
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT); // NACK last byte
        }

        else
        {
            // Current message complete
            twi_isr.message_count--;
            if (twi_isr.message_count > 0)
            {
                twi_isr.messages++;                                             // move to next message
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA); // repeated START
            }
            else
            {
                // All messages complete
                TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO); // STOP
                clear_twsto();
                twi_isr.status = SUCCESS;
                return_isr(SUCCESS);
            }
        }
        return;

    case TW_MR_DATA_NACK: // Data byte received, NACK transmitted
        *(buffer) = TWDR; // Store the last received byte
        buffer++;
        size--;

        twi_isr.message_count--;
        if (twi_isr.message_count > 0)
        {
            twi_isr.messages++;                                             // move to next message
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTA); // repeated START
        }
        else
        {
            // All messages complete
            TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO); // STOP
            clear_twsto();
            return_isr(SUCCESS);
        }
        return;

    case TW_MT_SLA_NACK:  // SLA+W transmitted, NACK received
    case TW_MR_SLA_NACK:  // SLA+R transmitted, NACK received
    case TW_MT_DATA_NACK: // Data byte transmitted, NACK received
        twi_isr.status = NACK_FAILURE;
        TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO); // STOP
        twi_isr.idle = true;
        return;

    default:
        TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT) | (1 << TWSTO);
        twi_isr.idle = true;
        clear_twsto();
        return;
    }
error:
    return_isr(status);
    TWCR |= 1 << TWSTO;
    TWCR |= 1 << TWINT; // Clear interrupt flag to exit ISR
    clear_twsto();
}