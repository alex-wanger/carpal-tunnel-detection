#include "calcal.h"

void calcal_init(calcal_t *s, uint32_t now_ms)
{
    s->sum_x = s->sum_y = s->sum_z = 0.0f;
    s->count = 0;
    s->t0_ms = now_ms;
}

void calcal_add(calcal_t *s, float ax, float ay, float az)
{
    s->sum_x += ax;
    s->sum_y += ay;
    s->sum_z += az;
    if (s->count != 0xFFFF)
        s->count++;
}

void calcal_compute_and_reset(calcal_t *s, uint32_t now_ms,
                              float *avg_x, float *avg_y, float *avg_z,
                              float *dt_s)
{
    uint32_t dt_ms = now_ms - s->t0_ms;
    *dt_s = dt_ms / 1000.0f;

    if (s->count > 0)
    {
        float inv = 1.0f / (float)s->count;
        *avg_x = s->sum_x * inv;
        *avg_y = s->sum_y * inv;
        *avg_z = s->sum_z * inv;
    }
    else
    {
        *avg_x = *avg_y = *avg_z = 0.0f;
    }

    // 重置为下一批
    s->sum_x = s->sum_y = s->sum_z = 0.0f;
    s->count = 0;
    s->t0_ms = now_ms;
}
