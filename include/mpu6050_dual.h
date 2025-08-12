#ifndef MPU6050_DUAL_H
#define MPU6050_DUAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "twi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPU6050_ADDR_1      0x68
#define MPU6050_ADDR_2      0x69

#define MPU_REG_SMPLRT_DIV  0x19
#define MPU_REG_CONFIG      0x1A
#define MPU_REG_GYRO_CFG    0x1B
#define MPU_REG_ACCEL_CFG   0x1C
#define MPU_REG_FIFO_EN     0x23
#define MPU_REG_USER_CTRL   0x6A
#define MPU_REG_PWR_MGMT_1  0x6B
#define MPU_REG_FIFO_COUNTH 0x72
#define MPU_REG_FIFO_R_W    0x74

#define MPU_BYTES_PER_SAMPLE 12
#define MPU_ACCEL_CONST      16384.0f

typedef struct { uint8_t addr1; uint8_t addr2; } mpu6050_dual_t;
typedef struct { int16_t ax, ay, az; int16_t gx, gy, gz; } mpu_fifo_sample_t;

twi_status_t mpu6050_write(uint8_t addr7, uint8_t reg, const uint8_t *data, size_t len);
twi_status_t mpu6050_read (uint8_t addr7, uint8_t reg, uint8_t *data, size_t len);
twi_status_t mpu6050_clear_fifo(uint8_t addr7);
twi_status_t mpu6050_read_fifo_bytes(uint8_t addr7, uint8_t *dst, size_t nbytes);
twi_status_t mpu6050_read_fifo_count(uint8_t addr7, uint16_t *count);
twi_status_t mpu6050_config_default(uint8_t addr7);

void         mpu6050_dual_init(mpu6050_dual_t *dev, uint8_t addr1, uint8_t addr2);
twi_status_t mpu6050_dual_config(mpu6050_dual_t *dev);
twi_status_t mpu6050_dual_clear_fifos(mpu6050_dual_t *dev);
twi_status_t mpu6050_dual_read_one(mpu6050_dual_t *dev, uint8_t which, uint8_t *dst, size_t max_bytes, size_t *out_bytes);

/* 兼容 main.c 现有调用 */
void mpu6050_dual_read(float* gx1,float* gy1,float* gz1, float* gx2,float* gy2,float* gz2);

#ifdef __cplusplus
}
#endif
#endif
