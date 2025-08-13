#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "twi.c"                // 保留你原来的写法
#include "../include/twi.h"
#include "timer.c"              // 保留你原来的写法
#include "../include/config.h"

// ==== 新增，仅包含头文件（源文件由 Makefile 单独编译）====
#include "../include/usart.h"
#include "../include/simpson.h"
#include "../include/cal.h"
// ==========================================================

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
        },
    };

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

    // ==== 新增：串口与 Simpson 存储 ====
    usart_init(115200);
    usart_print("USART Ready\r\n");

    simpson_store_t acceleration;   // 存原始三轴角速度（dps）
    simpson_store_t velocity;       // 存第一次积分的累计值（三轴）
    simpson_init(&acceleration);
    simpson_init(&velocity);

    uint32_t t0_accel = 0, t0_vel = 0;
    uint8_t  accel_started = 0, vel_started = 0;
    // =====================================

    uint32_t initial_time = millis();

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

                    int16_t gyro_x_raw = (sample[6] << 8) | sample[7];   // Bytes 6-7
                    int16_t gyro_y_raw = (sample[8] << 8) | sample[9];   // Bytes 8-9
                    int16_t gyro_z_raw = (sample[10] << 8) | sample[11]; // Bytes 10-11

                    volatile float accel_x_g = accel_x_raw / ACCEL_CONSTANT;
                    volatile float accel_y_g = accel_y_raw / ACCEL_CONSTANT;
                    volatile float accel_z_g = accel_z_raw / ACCEL_CONSTANT;

                    volatile float gyro_x_dps = gyro_x_raw / GYRO_CONSTANT;
                    volatile float gyro_y_dps = gyro_y_raw / GYRO_CONSTANT;
                    volatile float gyro_z_dps = gyro_z_raw / GYRO_CONSTANT;

                    // ====== 新增：串口打印 + 追加到 acceleration ======
                    usart_print("[GYRO] X=");
                    usart_print_float(gyro_x_dps, 2);
                    usart_print(", Y=");
                    usart_print_float(gyro_y_dps, 2);
                    usart_print(", Z=");
                    usart_print_float(gyro_z_dps, 2);
                    usart_print("\r\n");

                    simpson_append(&acceleration, gyro_x_dps, gyro_y_dps, gyro_z_dps);
                    if (!accel_started) { t0_accel = millis(); accel_started = 1; }
                    // ==================================================

                    // 原来的断点保持
                    volatile int YES = 1;
                }

                // ====== 新增：一次/二次 Simpson 积分并打印 ======
                uint16_t n_acc = simpson_size(&acceleration);
                if (n_acc >= 2)
                {
                    float total_s_acc = (millis() - t0_accel) / 1000.0f;
                    float dt_acc = total_s_acc / (float)(n_acc - 1);
                    if (dt_acc <= 0.f) dt_acc = 0.01f;

                    float vx = simpson_angle_x(&acceleration, dt_acc);
                    float vy = simpson_angle_y(&acceleration, dt_acc);
                    float vz = simpson_angle_z(&acceleration, dt_acc);

                    simpson_append(&velocity, vx, vy, vz);
                    if (!vel_started) { t0_vel = millis(); vel_started = 1; }

                    usart_print("[1st Integration] X=");
                    usart_print_float(vx, 2);
                    usart_print(", Y=");
                    usart_print_float(vy, 2);
                    usart_print(", Z=");
                    usart_print_float(vz, 2);
                    usart_print("\r\n");

                    uint16_t n_vel = simpson_size(&velocity);
                    if (n_vel >= 2)
                    {
                        float total_s_vel = (millis() - t0_vel) / 1000.0f;
                        float dt_vel = total_s_vel / (float)(n_vel - 1);
                        if (dt_vel <= 0.f) dt_vel = 0.01f;

                        float ang_x = simpson_angle_x(&velocity, dt_vel);
                        float ang_y = simpson_angle_y(&velocity, dt_vel);
                        float ang_z = simpson_angle_z(&velocity, dt_vel);

                        usart_print("[2nd Integration] X=");
                        usart_print_float(ang_x, 2);
                        usart_print(", Y=");
                        usart_print_float(ang_y, 2);
                        usart_print(", Z=");
                        usart_print_float(ang_z, 2);
                        usart_print("\r\n");
                    }
                }
                // =====================================================
            }
        }
    }

    return 0;
}
