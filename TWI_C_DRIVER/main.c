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

// Function to dump important MPU-6050 registers
void dump_accel_registers(void)
{
    // Accelerometer data registers
    struct
    {
        uint8_t addr;
        const char *name;
    } accel_regs[] = {
        {0x3B, "ACCEL_XOUT_H"},
        {0x3C, "ACCEL_XOUT_L"},
        {0x3D, "ACCEL_YOUT_H"},
        {0x3E, "ACCEL_YOUT_L"},
        {0x3F, "ACCEL_ZOUT_H"},
        {0x40, "ACCEL_ZOUT_L"}};

    static uint8_t accel_addresses[6];
    static uint8_t accel_values[6];
    static const char *accel_names[6];
    static int16_t accel_combined[3]; // Combined 16-bit values
    static float accel_g_values[3];   // Converted to g-force

    for (int i = 0; i < 6; i++)
    {
        accel_addresses[i] = accel_regs[i].addr;
        accel_values[i] = read_register(accel_regs[i].addr);
        accel_names[i] = accel_regs[i].name;

        // SET BREAKPOINT HERE to examine each raw register
        // In GDB: (gdb) print accel_names[i]
        //         (gdb) print/x accel_addresses[i]
        //         (gdb) print/x accel_values[i]
    }

    // Combine high and low bytes to form 16-bit values
    accel_combined[0] = (accel_values[0] << 8) | accel_values[1]; // ACCEL_X
    accel_combined[1] = (accel_values[2] << 8) | accel_values[3]; // ACCEL_Y
    accel_combined[2] = (accel_values[4] << 8) | accel_values[5]; // ACCEL_Z

    // Convert to g-force
    accel_g_values[0] = accel_combined[0] / ACCEL_CONSTANT; // X in g
    accel_g_values[1] = accel_combined[1] / ACCEL_CONSTANT; // Y in g
    accel_g_values[2] = accel_combined[2] / ACCEL_CONSTANT; // Z in g

    volatile int accel_dump_complete = 1; // SET BREAKPOINT HERE
    // In GDB: (gdb) print accel_combined[0]@3     # All raw 16-bit values
    //         (gdb) print accel_g_values[0]@3     # All g-force values
    //         (gdb) print/x accel_values[0]@6     # All raw register bytes
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

    dump_accel_registers();

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

    static uint8_t fifo_data_buffer[120];

    while (1)
    {
        clear_fifo();
        uint32_t current_time = millis();

        if (current_time - initial_time >= 500)
        {
            uint16_t fifo_count = calculate_fifo_bytes();
            initial_time = current_time;

            uint16_t samples = fifo_count / 12;
            if (samples > 10)
                samples = 10;
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