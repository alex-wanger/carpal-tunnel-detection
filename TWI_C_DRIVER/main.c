#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "twi.c"
#include "twi.h"
#include "timer.c"

uint8_t const MPU_ADDRESS = 0x68;
float const ACCEL_CONSTANT = 16384.0;

// Function to read a single register
uint8_t read_register(uint8_t reg_addr)
{
    static uint8_t reg_value;
    twi_message_t reg_read[2] = {
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = &reg_addr,
            .size = 1,
        },
        {
            .address = TWI_READ_ADDRESS(MPU_ADDRESS),
            .buffer = &reg_value,
            .size = 1,
        }};

    twi_enqueue(reg_read, 2);
    while (!twi_isr.idle)
        ;

    return reg_value;
}

void clear_fifo(void)
{
    twi_message_t CLEAR_FIFO[3] = {
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x6A, 0b00000100},
            .size = 2,
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x6A, 0b01000000},
            .size = 2,
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x23, 0b01111000},
            .size = 2,
        }};

    twi_enqueue(CLEAR_FIFO, 3);
    while (!twi_isr.idle)
        ;
}

uint16_t calculate_fifo_bytes(void)
{
    static uint8_t fifo_count_buffer[2];
    twi_message_t fifo_count_read[2] = {
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x72},
            .size = 1,
        },
        {
            .address = TWI_READ_ADDRESS(MPU_ADDRESS),
            .buffer = fifo_count_buffer,
            .size = 2,
        }};

    twi_enqueue(fifo_count_read, 2);
    while (!twi_isr.idle)
        ;

    uint16_t fifo_bytes = (fifo_count_buffer[0] << 8) | fifo_count_buffer[1];
    return fifo_bytes;
}

int main(void)
{
    sei();
    twi_status_t status = twi_init(100000);

    uint32_t initial_time = millis();

    while (status != SUCCESS)
        ;

    twi_message_t CONFIG[7] = {
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x19, 0x31},
            .size = 2,
            // CONFIGURE SAMPLE RATE (1000 / (49+1) == 20Hz)
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x6B, 0x00},
            .size = 2,
            // WAKE UP MPU
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x1A, 0x06},
            .size = 2,
            // CONFIGURE DLPF 5Hz
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x1B, 0x00},
            .size = 2,
            // GYRO_CONFIG: ±250°/s range
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x1C, 0x00},
            .size = 2,
            // ACCEL_CONFIG: ±2g range
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x6A, 0b01000000},
            .size = 2,
            // USER_CTRL: ENABLE FIFO
        },
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x23, 0b01111000},
            .size = 2,
            // FIFO_EN: Enable GYRO_X,Y,Z + ACCEL
        },
    };

    twi_enqueue(CONFIG, 7);
    while (!twi_isr.idle)
        ;

    static uint8_t fifo_data_buffer[60];

    while (1)
    {
        clear_fifo();
        uint32_t current_time = millis();

        if (current_time - initial_time >= 250)
        {
            uint16_t fifo_count = calculate_fifo_bytes();
            initial_time = current_time;

            uint16_t samples = fifo_count / 12;
            if (samples > 5)
                samples = 5;
            uint16_t bytes_to_read = samples * 12;

            if (samples > 0)
            {
                twi_message_t fifo_data_read[2] = {
                    {
                        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
                        .buffer = (uint8_t[]){0x74},
                        .size = 1,
                    },
                    {
                        .address = TWI_READ_ADDRESS(MPU_ADDRESS),
                        .buffer = fifo_data_buffer,
                        .size = bytes_to_read,
                    }};

                twi_enqueue(fifo_data_read, 2);
                while (!twi_isr.idle)
                    ;

                for (size_t i = 0; i < samples; i++)
                {
                    uint8_t *sample = &fifo_data_buffer[i * 12];

                    int16_t accel_x_raw = (sample[0] << 8) | sample[1]; // Bytes 0-1
                    int16_t accel_y_raw = (sample[2] << 8) | sample[3]; // Bytes 2-3
                    int16_t accel_z_raw = (sample[4] << 8) | sample[5]; // Bytes 4-5

                    volatile float accel_x_g = accel_x_raw / ACCEL_CONSTANT;
                    volatile float accel_y_g = accel_y_raw / ACCEL_CONSTANT;
                    volatile float accel_z_g = accel_z_raw / ACCEL_CONSTANT;

                    // SET BREAKPOINT HERE to verify the values
                    volatile int YES = 1;
                }
            }
        }
    }

    return 0;
}