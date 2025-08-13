#include "twi.h"

float const ACCEL_CONSTANT = 16384.0;
float const GYRO_CONSTANT = 131.0;

const uint8_t MPU_ADDRESS_LOW = 0x68;
const uint8_t MPU_ADDRESS_HIGH = 0x69;

twi_message_t CONFIG[14] = {
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x19, 0x31},
        .size = 2,
        // CONFIGURE SAMPLE RATE (1000 / (49+1) == 20Hz)
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x6B, 0x00},
        .size = 2,
        // WAKE UP MPU
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x1A, 0x06},
        .size = 2,
        // CONFIGURE DLPF 5Hz
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x1B, 0x00},
        .size = 2,
        // GYRO_CONFIG: ±250°/s range
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x1C, 0x00},
        .size = 2,
        // ACCEL_CONFIG: ±2g range
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x6A, 0b01000100},
        .size = 2,
        // USER_CTRL: ENABLE FIFO
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_LOW),
        .buffer = (uint8_t[]){0x23, 0b00001000},
        .size = 2,
        // FIFO_EN: Enable GYRO_X,Y,Z + ACCEL
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x19, 0x31},
        .size = 2,
        // CONFIGURE SAMPLE RATE (1000 / (49+1) == 20Hz)
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x6B, 0x00},
        .size = 2,
        // WAKE UP MPU
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x1A, 0x06},
        .size = 2,
        // CONFIGURE DLPF 5Hz
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x1B, 0x00},
        .size = 2,
        // GYRO_CONFIG: ±250°/s range
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x1C, 0x00},
        .size = 2,
        // ACCEL_CONFIG: ±2g range
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x6A, 0b01000100},
        .size = 2,
        // USER_CTRL: ENABLE FIFO
    },
    {
        .address = TWI_WRITE_ADDRESS(MPU_ADDRESS_HIGH),
        .buffer = (uint8_t[]){0x23, 0b00001000},
        .size = 2,
        // FIFO_EN: Enable GYRO_X,Y,Z + ACCEL
    },
};
