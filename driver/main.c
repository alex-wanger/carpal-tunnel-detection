#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include "twi.c"
#include "../include/twi.h"
#include "timer.c"
#include "../include/config.h"
#include "vector.c"
#include <util/delay.h>
void read_accel_registers(uint8_t MPU_ADDRESS, float *accel_result)
{
    static uint8_t accel_data[6];

    twi_message_t accel_read[2] = {
        {
            .address = TWI_WRITE_ADDRESS(MPU_ADDRESS),
            .buffer = (uint8_t[]){0x3B}, // Start at ACCEL_XOUT_H register
            .size = 1,
        },
        {
            .address = TWI_READ_ADDRESS(MPU_ADDRESS),
            .buffer = accel_data,
            .size = 6, // Read 6 bytes (X, Y, Z - 2 bytes each)
        }};

    twi_enqueue(accel_read, 2);
    while (!twi_isr.idle)
        ;

    // Combine high and low bytes for each axis
    int16_t accel_x_raw = (accel_data[0] << 8) | accel_data[1];
    int16_t accel_y_raw = (accel_data[2] << 8) | accel_data[3];
    int16_t accel_z_raw = (accel_data[4] << 8) | accel_data[5];

    // Convert to g-force
    accel_result[0] = (float)accel_x_raw / ACCEL_CONSTANT;
    accel_result[1] = (float)accel_y_raw / ACCEL_CONSTANT;
    accel_result[2] = (float)accel_z_raw / ACCEL_CONSTANT;
}

int main(void)
{
    float HIGH_ACCEL[3] = {0.0, 0.0, 0.0};
    float LOW_ACCEL[3] = {0.0, 0.0, 0.0};
    float dot_product = 0.0;

    twi_status_t status = twi_init(100000);

    if (!twi_isr.idle)
    {
        while (1)
            ;
    }

    uint32_t initial_time = millis();

    static uint32_t led_timer = 0;
    static bool led_active = false;
    static bool led_initialized = false;

    twi_enqueue(CONFIG, 14);
    while (!twi_isr.idle)
        ;

    uint8_t mpu_addresses[2] = {0x69, 0x68};

    while (1)
    {
        uint32_t current_time = millis();

        if (current_time - initial_time >= 250)
        {
            initial_time = current_time;

            read_accel_registers(mpu_addresses[0], HIGH_ACCEL);

            read_accel_registers(mpu_addresses[1], LOW_ACCEL);

            vec3 high_vec = vec3_make(HIGH_ACCEL[0], HIGH_ACCEL[1], HIGH_ACCEL[2]);
            vec3 low_vec = vec3_make(LOW_ACCEL[0], LOW_ACCEL[1], LOW_ACCEL[2]);
            dot_product = vec_dot(high_vec, low_vec);

            int break_flag = 0;

            if (dot_product < 0.65)
            {
                if (!led_initialized)
                {
                    DDRD |= (1 << PD4) | (1 << PD5); // initalize as output
                    PORTD |= (1 << PD4);
                    PORTD &= ~(1 << PD5);
                    led_timer = millis();
                    led_initialized = true;
                    led_active = true;
                }

                if (led_active && (millis() - led_timer >= 150))
                {
                    PORTD ^= (1 << PD4) | (1 << PD5);
                    led_timer = millis();
                }
            }
            else
            {
                led_active = false;
                led_initialized = false;
                PORTD &= ~((1 << PD4) | (1 << PD5));
            }
        }
    }
}