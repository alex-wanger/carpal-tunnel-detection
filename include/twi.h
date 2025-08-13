#ifndef TWI_DRIVER_H
#define TWI_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/twi.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TWI_WRITE_ADDRESS(addr7) (uint8_t)(((addr7) << 1) | 0x00)
#define TWI_READ_ADDRESS(addr7) (uint8_t)(((addr7) << 1) | 0x01)

    typedef struct
    {
        uint8_t address;
        uint8_t *buffer;
        size_t size;
    } twi_message_t;

    typedef enum
    {
        SUCCESS = 0,
        INIT_FAILURE,
        DISABLED,
        BUSY,
        NACK_FAILURE,
        OVERWRITE_FAILURE,
        FAILURE
    } twi_status_t;

    typedef struct
    {
        volatile bool idle;
        volatile twi_message_t *messages;
        volatile size_t message_count;
        volatile twi_status_t status;
    } twi_isr_t;

    static volatile twi_isr_t twi_isr;

    twi_status_t twi_init(uint32_t scl_frequency);
    twi_status_t twi_enqueue(twi_message_t *messages, size_t count);
    twi_status_t twi_status(void);

#ifdef __cplusplus
}
#endif
#endif
