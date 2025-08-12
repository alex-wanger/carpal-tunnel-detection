#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "../include/twi.h"
#include "../include/config.h"
#include "timer.c"
#include "usart.c"
#include "twi.c"
#include "simpson.c"
#include "../include/simpson.h"
#include "cal.c"

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
    twi_init(100000);

    uint32_t initial_time = millis();
    uint32_t last_integration_time = initial_time;
    simpson_store_t simps;

    twi_enqueue(CONFIG, 7);
    while (!twi_isr.idle)
        ;

    static uint8_t fifo_data_buffer[60];

    while (1)
    {
        uint32_t current_time = millis();
        // clear_fifo();
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

                    // int16_t gyro_x_raw = (sample[6] << 8) | sample[7];   // Bytes 6-7
                    // int16_t gyro_y_raw = (sample[8] << 8) | sample[9];   // Bytes 8-9
                    // int16_t gyro_z_raw = (sample[10] << 8) | sample[11]; // Bytes 10-11

                    volatile float accel_x_g = accel_x_raw / ACCEL_CONSTANT;
                    volatile float accel_y_g = accel_y_raw / ACCEL_CONSTANT;
                    volatile float accel_z_g = accel_z_raw / ACCEL_CONSTANT;

                    // volatile float gyro_x_dps = gyro_x_raw / GYRO_CONSTANT;
                    // volatile float gyro_y_dps = gyro_y_raw / GYRO_CONSTANT;
                    // volatile float gyro_z_dps = gyro_z_raw / GYRO_CONSTANT;

                    // SET BREAKPOINT HERE to verify the values
                    volatile int YES = 1;
                    // simpson_append(&simps, gyro_x_dps, gyro_y_dps, gyro_z_dps);
                }

                // if (simpson_size(&simps) >= 2)
                // {
                //     float dt_per_sample = (float)(current_time - last_integration_time) / 1000.0f;
                //     if (samples > 0)
                //         dt_per_sample /= (float)samples;
                //     if (dt_per_sample <= 0.0f)
                //         dt_per_sample = 0.01f;

                //     float angle_x_deg = simpson_angle_x(&simps, dt_per_sample);
                //     float angle_y_deg = simpson_angle_y(&simps, dt_per_sample);
                //     float angle_z_deg = simpson_angle_z(&simps, dt_per_sample);

                //     last_integration_time = current_time;
                // }
            }
        }
    }

    return 0;
}