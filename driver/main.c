#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "twi.c"
#include "../include/twi.h"
#include "timer.c"
#include "../include/config.h"

#include "../include/simpson.h"
#include "simpson.c"
#include "cal.c"
#include "../include/cal.h"

#include "../include/vector.h"    // vector
#include "../include/led.h"       // led
#include "../include/buzzer.h"    // buzzer
#include "../include/usart.h"     // print

void clear_fifo(uint8_t MPU_ADDRESS)
{
    twi_message_t CLEAR_FIFO[3] = {
        { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x6A, 0b00000100}, .size = 2, },
        { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x6A, 0b01000000}, .size = 2, },
        { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x23, 0b01111000}, .size = 2, },
    };
    twi_enqueue(CLEAR_FIFO, 3);
    while (!twi_isr.idle) {;}
}

uint16_t calculate_fifo_bytes(uint8_t MPU_ADDRESS)
{
    static uint8_t fifo_count_buffer[2];
    twi_message_t fifo_count_read[2] = {
        { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x72}, .size = 1, },
        { .address = TWI_READ_ADDRESS(MPU_ADDRESS),  .buffer = fifo_count_buffer, .size = 2, },
    };
    twi_enqueue(fifo_count_read, 2);
    while (!twi_isr.idle) {;}
    uint16_t fifo_bytes = (fifo_count_buffer[0] << 8) | fifo_count_buffer[1];
    return fifo_bytes;
}

int main(void)
{
    float HIGH_ACCEL[3];
    float LOW_ACCEL[3];

    float DIFF[3];                        // led/buzzer
    float wrist_deg = NAN;                // vector
    bool have_high = false, have_low = false;

    sei();
    twi_status_t status = twi_init(100000);

    // print
    usart_init(115200);

    uint32_t initial_time = millis();
    uint32_t last_print_ms = 0;          // print 2.5s

    twi_enqueue(CONFIG, 14);
    while (!twi_isr.idle) {;}

    static uint8_t fifo_data_buffer[30];
    uint8_t mpu_addresses[2] = {0x69, 0x68};

    led_init();                           // led
    buzzer_init();                        // buzzer

    while (1)
    {
        uint32_t current_time = millis();

        if (current_time - initial_time >= 250)
        {
            initial_time = current_time;

            for (uint8_t mpu_index = 0; mpu_index < 2; mpu_index++)
            {
                uint8_t MPU_ADDRESS = mpu_addresses[mpu_index];

                uint16_t fifo_count = calculate_fifo_bytes(MPU_ADDRESS);

                if (fifo_count >= 1024)
                {
                    clear_fifo(MPU_ADDRESS);
                    fifo_count = 0;
                    continue;
                }

                uint16_t samples = fifo_count / 6;
                if (samples > 5) samples = 5;
                uint16_t bytes_to_read = samples * 6;

                if (samples > 0)
                {
                    twi_message_t fifo_data_read[2] = {
                        { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x74}, .size = 1, },
                        { .address = TWI_READ_ADDRESS(MPU_ADDRESS),  .buffer = fifo_data_buffer,   .size = bytes_to_read, },
                    };
                    twi_enqueue(fifo_data_read, 2);
                    while (!twi_isr.idle) {;}

                    for (size_t i = 0; i < samples; i++)
                    {
                        uint8_t *sample = &fifo_data_buffer[i * 12];

                        int16_t accel_x_raw = (sample[0] << 8) | sample[1];
                        int16_t accel_y_raw = (sample[2] << 8) | sample[3];
                        int16_t accel_z_raw = (sample[4] << 8) | sample[5];

                        volatile float accel_x_g = accel_x_raw / ACCEL_CONSTANT;
                        volatile float accel_y_g = accel_y_raw / ACCEL_CONSTANT;
                        volatile float accel_z_g = accel_z_raw / ACCEL_CONSTANT;

                        if (mpu_index == 0) {
                            HIGH_ACCEL[0] = accel_x_g;
                            HIGH_ACCEL[1] = accel_y_g;
                            HIGH_ACCEL[2] = accel_z_g;
                            have_high = true;
                        } else {
                            LOW_ACCEL[0] = accel_x_g;
                            LOW_ACCEL[1] = accel_y_g;
                            LOW_ACCEL[2] = accel_z_g;
                            have_low  = true;
                        }
                    }
                }
            }

            if (have_high && have_low) {
                DIFF[0] = LOW_ACCEL[0] - HIGH_ACCEL[0];
                DIFF[1] = LOW_ACCEL[1] - HIGH_ACCEL[1];
                DIFF[2] = LOW_ACCEL[2] - HIGH_ACCEL[2];

                wrist_deg = vec_angle_deg_a(HIGH_ACCEL, LOW_ACCEL);

                led_update(DIFF[0], DIFF[1], DIFF[2]);       // led
                buzzer_update(DIFF[0], DIFF[1], DIFF[2]);    // buzzer

                // print every 2.5s
                if (millis() - last_print_ms >= 2500) {
                    usart_print("HIGH: ");
                    usart_print_float(HIGH_ACCEL[0], 2); usart_print(", ");
                    usart_print_float(HIGH_ACCEL[1], 2); usart_print(", ");
                    usart_print_float(HIGH_ACCEL[2], 2); usart_print("  ");

                    usart_print("LOW: ");
                    usart_print_float(LOW_ACCEL[0], 2);  usart_print(", ");
                    usart_print_float(LOW_ACCEL[1], 2);  usart_print(", ");
                    usart_print_float(LOW_ACCEL[2], 2);  usart_print("  ");

                    usart_print("ANGLE: ");
                    usart_print_float(wrist_deg, 2);
                    usart_print("\r\n");

                    last_print_ms = millis();
                }

                have_high = have_low = false;
            }
        }
    }
}
