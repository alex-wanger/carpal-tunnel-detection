#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "twi.c"
#include "twi.h"

int main(void)
{
    sei();
    twi_status_t status = twi_init(100000);

    while (status != SUCCESS)
        ;

    twi_message_t message1[2] = {
        {
            .address = TWI_WRITE_ADDRESS(0x68),
            .buffer = (uint8_t[]){0x1A, 0x05},
            .size = 2,
        },
        {
            .address = TWI_READ_ADDRESS(0x1A),
            .buffer = (uint8_t[]){0},
            .size = 1,
        }};

    twi_message_t message2[2] = {
        {
            .address = TWI_WRITE_ADDRESS(0x68),
            .buffer = (uint8_t[]){0x1B, 0x00},
            .size = 2,
        },
        {
            .address = TWI_READ_ADDRESS(0x1B),
            .buffer = (uint8_t[]){0},
            .size = 1,
        }};

    twi_enqueue(message1, 2);

    while (!twi_isr.idle)
        ;
    twi_enqueue(message2, 2);

    {
    }

    while (1)
        ;
    return 0;
}