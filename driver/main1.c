// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <stdint.h>

// #include "twi.c"
// #include "../include/twi.h"
// #include "timer.c"
// #include "../include/config.h"

// #include "../include/usart.h"
// #include "calcal.h"

// uint8_t read_register(uint8_t reg_addr)
// {
//     static uint8_t reg_value;
//     twi_message_t reg_read[2] = {
//         { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = &reg_addr, .size = 1, },
//         { .address = TWI_READ_ADDRESS(MPU_ADDRESS),  .buffer = &reg_value, .size = 1, },
//     };
//     twi_enqueue(reg_read, 2);
//     while (!twi_isr.idle) ;
//     return reg_value;
// }

// void clear_fifo(void)
// {
//     twi_message_t CLEAR_FIFO[3] = {
//         { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x6A, 0b00000100}, .size = 2, },
//         { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x6A, 0b01000000}, .size = 2, },
//         { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x23, 0b01111000}, .size = 2, },
//     };
//     twi_enqueue(CLEAR_FIFO, 3);
//     while (!twi_isr.idle) ;
// }

// uint16_t calculate_fifo_bytes(void)
// {
//     static uint8_t fifo_count_buffer[2];
//     twi_message_t fifo_count_read[2] = {
//         { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x72}, .size = 1, },
//         { .address = TWI_READ_ADDRESS(MPU_ADDRESS),  .buffer = fifo_count_buffer, .size = 2,    },
//     };
//     twi_enqueue(fifo_count_read, 2);
//     while (!twi_isr.idle) ;
//     return (uint16_t)((fifo_count_buffer[0] << 8) | fifo_count_buffer[1]);
// }

// int main(void)
// {
//     sei();
//     (void)twi_init(100000);

//     usart_init(115200);
//     usart_print("USART Ready (calcal)\r\n");

//     float angle_x = 0.0f, angle_y = 0.0f, angle_z = 0.0f;

//     calcal_t cal;
//     calcal_init(&cal, millis());

//     uint32_t tick = millis();

//     twi_enqueue(CONFIG, 7);
//     while (!twi_isr.idle) ;

//     static uint8_t fifo_data_buffer[60];

//     while (1)
//     {
//         clear_fifo();
//         uint32_t now = millis();

//         if (now - tick >= 250)
//         {
//             uint16_t fifo_count = calculate_fifo_bytes();
//             tick = now;

//             uint16_t samples = fifo_count / 12;
//             if (samples > 5) samples = 5;
//             uint16_t bytes_to_read = samples * 12;

//             if (samples > 0)
//             {
//                 twi_message_t fifo_data_read[2] = {
//                     { .address = TWI_WRITE_ADDRESS(MPU_ADDRESS), .buffer = (uint8_t[]){0x74}, .size = 1, },
//                     { .address = TWI_READ_ADDRESS(MPU_ADDRESS),  .buffer = fifo_data_buffer,   .size = bytes_to_read, },
//                 };

//                 twi_enqueue(fifo_data_read, 2);
//                 while (!twi_isr.idle) ;

//                 for (size_t i = 0; i < samples; i++)
//                 {
//                     uint8_t *sample = &fifo_data_buffer[i * 12];

//                     int16_t accel_x_raw = (sample[0]  << 8) | sample[1];
//                     int16_t accel_y_raw = (sample[2]  << 8) | sample[3];
//                     int16_t accel_z_raw = (sample[4]  << 8) | sample[5];

//                     float ax = accel_x_raw / ACCEL_CONSTANT;
//                     float ay = accel_y_raw / ACCEL_CONSTANT;
//                     float az = accel_z_raw / ACCEL_CONSTANT;

//                     calcal_add(&cal, ax, ay, az);
//                 }

//                 float avg_x, avg_y, avg_z, dt;
//                 calcal_compute_and_reset(&cal, millis(), &avg_x, &avg_y, &avg_z, &dt);

//                 float omega_x = avg_x * dt;
//                 float omega_y = avg_y * dt;
//                 float omega_z = avg_z * dt;

//                 angle_x += omega_x * dt;
//                 angle_y += omega_y * dt;
//                 angle_z += omega_z * dt;

//                 usart_print("[avg a] X="); usart_print_float(avg_x, 3);
//                 usart_print(", Y=");       usart_print_float(avg_y, 3);
//                 usart_print(", Z=");       usart_print_float(avg_z, 3);
//                 usart_print(", dt=");      usart_print_float(dt, 3);
//                 usart_print(" s\r\n");

//                 usart_print("[omega]  X="); usart_print_float(omega_x, 3);
//                 usart_print(", Y=");        usart_print_float(omega_y, 3);
//                 usart_print(", Z=");        usart_print_float(omega_z, 3);
//                 usart_print("\r\n");

//                 usart_print("[angle]  X="); usart_print_float(angle_x, 3);
//                 usart_print(", Y=");        usart_print_float(angle_y, 3);
//                 usart_print(", Z=");        usart_print_float(angle_z, 3);
//                 usart_print("\r\n");
//             }
//         }
//     }

//     return 0;
// }
