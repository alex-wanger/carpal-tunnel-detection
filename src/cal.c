#include "cal.h"

static float simpson_calc(float *arr, uint16_t n, float dt) {
    if (n < 2) return 0.0f;

    float sum = arr[0] + arr[n - 1]; 

    for (uint16_t i = 1; i < n - 1; i++) {
        if (i % 2 == 0) {
            sum += 4 * arr[i];
        } else {
            sum += 2 * arr[i];
        }
    }

    return (dt / 3.0f) * sum;
}

float simpson_angle_x(simpson_store_t *s, float dt) {
    return simpson_calc(s->x, s->size, dt);
}

float simpson_angle_y(simpson_store_t *s, float dt) {
    return simpson_calc(s->y, s->size, dt);
}

float simpson_angle_z(simpson_store_t *s, float dt) {
    return simpson_calc(s->z, s->size, dt);
}
