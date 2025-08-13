#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include "twi.c"
#include "../include/twi.h"
#include "timer.c"
#include "../include/config.h"
#include "cal.c"
#include "vector.c"
#include "../include/vector.h"
#include "../include/cal.h"

void clear_fifo(uint8_t MPU_ADDRESS)
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
        },
    };

    twi_enqueue(CLEAR_FIFO, 3);
    while (!twi_isr.idle)
        ;
}

uint8_t read_register(uint8_t mpu_address, uint8_t reg_addr)
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

uint16_t calculate_fifo_bytes(uint8_t MPU_ADDRESS)
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

// int main(void)
// {
//     float HIGH_ACCEL[3];
//     float LOW_ACCEL[3];

//     twi_status_t status = twi_init(100000);

//     uint32_t initial_time = millis();

//     twi_enqueue(CONFIG, 14);
//     while (!twi_isr.idle)
//         ;

//     static uint8_t fifo_data_buffer[30];
//     uint8_t mpu_addresses[2] = {0x69, 0x68};

//     while (1)
//     {
//         uint32_t current_time = millis();

//         if (current_time - initial_time >= 250)
//         {
//             initial_time = current_time;

//             for (uint8_t mpu_index = 0; mpu_index < 2; mpu_index++)
//             {
//                 float SUM_DOT_PRODUCT = 0.0;

//                 read_register(mpu_addresses[mpu_index], 0x75);

//                 // uint8_t MPU_ADDRESS = mpu_addresses[mpu_index];

//                 // uint16_t fifo_count = calculate_fifo_bytes(MPU_ADDRESS);

//                 // if (fifo_count >= 1020)
//                 // {
//                 //     clear_fifo(MPU_ADDRESS);
//                 //     fifo_count = 0;
//                 //     continue;
//                 // }

//                 // uint16_t samples = fifo_count / 6;

//                 // if (samples > 5)
//                 //     samples = 5;

//                 // uint16_t bytes_to_read = samples * 6;

//                 // if (samples > 0)
//                 // {
//                 //     twi_message_t fifo_data_read[2] = {
//                 //         {
//                 //             .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
//                 //             .buffer = (uint8_t[]){0x74},
//                 //             .size = 1,
//                 //         },
//                 //         {
//                 //             .address = TWI_READ_ADDRESS(MPU_ADDRESS),
//                 //             .buffer = fifo_data_buffer,
//                 //             .size = bytes_to_read,
//                 //         }};

//                 //     while (!twi_isr.idle)
//                 //         ;

//                 //     for (size_t i = 0; i < samples; i++)
//                 //     {
//                 // uint8_t *sample = &fifo_data_buffer[i * 6];

//                 int16_t accel_x_raw = (sample[0] << 8) | sample[1];
//                 int16_t accel_y_raw = (sample[2] << 8) | sample[3];
//                 int16_t accel_z_raw = (sample[4] << 8) | sample[5];

//                 float accel_x_g = (float)accel_x_raw / ACCEL_CONSTANT;
//                 float accel_y_g = (float)accel_y_raw / ACCEL_CONSTANT;
//                 float accel_z_g = (float)accel_z_raw / ACCEL_CONSTANT;

//                 if (mpu_index == 0)
//                 {
//                     HIGH_ACCEL[0] += accel_x_g;
//                     HIGH_ACCEL[1] += accel_y_g;
//                     HIGH_ACCEL[2] += accel_z_g;
//                 }
//                 else
//                 {
//                     LOW_ACCEL[0] += accel_x_g;
//                     LOW_ACCEL[1] += accel_y_g;
//                     LOW_ACCEL[2] += accel_z_g;
//                 }
//                 // }
//                 // }
//                 // vec3 high_vec = vec3_make(HIGH_ACCEL[0] / samples, HIGH_ACCEL[1] / samples, HIGH_ACCEL[2] / samples);
//                 // vec3 low_vec = vec3_make(LOW_ACCEL[0] / samples, LOW_ACCEL[1] / samples, LOW_ACCEL[2] / samples);
//                 // float dot_product = vec_dot(high_vec, low_vec);

//                 int BREAK = 1;
//             }
//         }
//     }
// }