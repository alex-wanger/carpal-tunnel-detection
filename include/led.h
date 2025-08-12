#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <avr/io.h>

void led_init(void);

void led_update(float x, float y, float z);

#endif
