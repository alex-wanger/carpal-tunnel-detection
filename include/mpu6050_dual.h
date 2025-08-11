#ifndef MPU6050_DUAL_H
#define MPU6050_DUAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPU6050_ADDR1 0x68  // AD0 = GND
#define MPU6050_ADDR2 0x69  // AD0 = VCC

void mpu6050_dual_init(void);
void mpu6050_dual_read(float *gx1, float *gy1, float *gz1,
                       float *gx2, float *gy2, float *gz2);

#ifdef __cplusplus
}
#endif

#endif // MPU6050_DUAL_H
