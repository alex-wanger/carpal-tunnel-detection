#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { float x, y, z; } vec3;

#ifndef VEC_EPS
#define VEC_EPS 1e-6f
#endif

static inline vec3 vec3_make(float x, float y, float z) {
    vec3 v = { x, y, z };
    return v;
}

float vec_dot(vec3 a, vec3 b);
vec3  vec_cross(vec3 a, vec3 b);
float vec_norm(vec3 a);
vec3  vec_normalize(vec3 v);

float vec_angle_rad(vec3 a, vec3 b);
float vec_angle_deg(vec3 a, vec3 b);

float vec_angle_rad_a(const float a[3], const float b[3]);
float vec_angle_deg_a(const float a[3], const float b[3]);

#ifdef __cplusplus
}
#endif

#endif
