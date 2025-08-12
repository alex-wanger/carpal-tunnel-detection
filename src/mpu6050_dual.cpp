#include "mpu6050_dual.h"

// ---- single-device helpers (stubs) ----
twi_status_t mpu6050_write(uint8_t addr7, uint8_t reg, const uint8_t *data, size_t len) {
    (void)addr7; (void)reg; (void)data; (void)len;
    return SUCCESS;
}

twi_status_t mpu6050_read(uint8_t addr7, uint8_t reg, uint8_t *data, size_t len) {
    (void)addr7; (void)reg; (void)data; (void)len;
    return SUCCESS;
}

twi_status_t mpu6050_clear_fifo(uint8_t addr7) {
    (void)addr7;
    return SUCCESS;
}

twi_status_t mpu6050_read_fifo_bytes(uint8_t addr7, uint8_t *dst, size_t nbytes) {
    (void)addr7; (void)dst; (void)nbytes;
    return SUCCESS;
}

twi_status_t mpu6050_read_fifo_count(uint8_t addr7, uint16_t *count) {
    (void)addr7; (void)count;
    return SUCCESS;
}

twi_status_t mpu6050_config_default(uint8_t addr7) {
    (void)addr7;
    return SUCCESS;
}

// ---- dual-device facade (stubs) ----
void mpu6050_dual_init(mpu6050_dual_t *dev, uint8_t addr1, uint8_t addr2) {
    if (!dev) return;
    dev->addr1 = addr1;
    dev->addr2 = addr2;
}

twi_status_t mpu6050_dual_config(mpu6050_dual_t *dev) {
    (void)dev;
    return SUCCESS;
}

twi_status_t mpu6050_dual_clear_fifos(mpu6050_dual_t *dev) {
    (void)dev;
    return SUCCESS;
}

twi_status_t mpu6050_dual_read_one(mpu6050_dual_t *dev, uint8_t which, uint8_t *dst, size_t max_bytes, size_t *out_bytes) {
    (void)dev; (void)which; (void)dst; (void)max_bytes; (void)out_bytes;
    return SUCCESS;
}

#ifdef __cplusplus
extern "C" {
#endif

void mpu6050_dual_read(float* gx1,float* gy1,float* gz1,
                       float* gx2,float* gy2,float* gz2)
{
    if (gx1) *gx1 = 0.0f;
    if (gy1) *gy1 = 0.0f;
    if (gz1) *gz1 = 0.0f;
    if (gx2) *gx2 = 0.0f;
    if (gy2) *gy2 = 0.0f;
    if (gz2) *gz2 = 0.0f;
}

#ifdef __cplusplus
}
#endif

