#ifndef SIMPSON_H
#define SIMPSON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float *x;
    float *y;
    float *z;
    uint16_t size;
    uint16_t capacity;
} simpson_store_t;

#ifndef SIMPSON_START_CAP
#define SIMPSON_START_CAP 32
#endif

void simpson_init(simpson_store_t *s);
void simpson_clear(simpson_store_t *s);
void simpson_free(simpson_store_t *s);
uint8_t simpson_append(simpson_store_t *s, float gx, float gy, float gz);

static inline uint16_t simpson_size(simpson_store_t *s) { return s->size; }
static inline const float* simpson_data_x(const simpson_store_t *s) { return s->x; }
static inline const float* simpson_data_y(const simpson_store_t *s) { return s->y; }
static inline const float* simpson_data_z(const simpson_store_t *s) { return s->z; }

#ifdef __cplusplus
}
#endif

#endif
