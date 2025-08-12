#ifndef CAL_H
#define CAL_H

#include <stdint.h>
#include "simpson.h"

float simpson_angle_x(simpson_store_t *s, float dt);
float simpson_angle_y(simpson_store_t *s, float dt);
float simpson_angle_z(simpson_store_t *s, float dt);

#endif
