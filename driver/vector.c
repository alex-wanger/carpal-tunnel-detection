#include "vector.h"

static inline float clamp_pm1(float x) {
    if (x > 1.0f)  return 1.0f;
    if (x < -1.0f) return -1.0f;
    return x;
}

float vec_dot(vec3 a, vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

vec3 vec_cross(vec3 a, vec3 b) {
    vec3 c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

float vec_norm(vec3 a) {
    return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

vec3 vec_normalize(vec3 v) {
    float n = vec_norm(v);
    if (n < VEC_EPS) {
        return v;
    }
    float inv = 1.0f / n;
    vec3 r = { v.x*inv, v.y*inv, v.z*inv };
    return r;
}

float vec_angle_rad(vec3 a, vec3 b) {
    float na = vec_norm(a);
    float nb = vec_norm(b);
    if (na < VEC_EPS || nb < VEC_EPS) {
        return NAN;
    }
    float cosv = vec_dot(a, b) / (na * nb);
    cosv = clamp_pm1(cosv);
    return acosf(cosv);
}

float vec_angle_deg(vec3 a, vec3 b) {
    float rad = vec_angle_rad(a, b);
    if (isnan(rad)) return NAN;
    return rad * (180.0f / (float)M_PI);
}

float vec_angle_rad_a(const float a[3], const float b[3]) {
    vec3 va = { a[0], a[1], a[2] };
    vec3 vb = { b[0], b[1], b[2] };
    return vec_angle_rad(va, vb);
}

float vec_angle_deg_a(const float a[3], const float b[3]) {
    vec3 va = { a[0], a[1], a[2] };
    vec3 vb = { b[0], b[1], b[2] };
    return vec_angle_deg(va, vb);
}
