#ifndef CALCAL_H
#define CALCAL_H

#include <stdint.h>

typedef struct {
    float     sum_x, sum_y, sum_z;
    uint16_t  count;
    uint32_t  t0_ms;
} calcal_t;

void calcal_init(calcal_t *s, uint32_t now_ms);

void calcal_add(calcal_t *s, float ax, float ay, float az);

void calcal_compute_and_reset(calcal_t *s, uint32_t now_ms,
                              float *avg_x, float *avg_y, float *avg_z,
                              float *dt_s);

#endif
