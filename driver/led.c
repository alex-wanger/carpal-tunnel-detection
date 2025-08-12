#include "led.h"

#define LED_X PD3
#define LED_Y PD4
#define LED_Z PD5

void led_init(void) {
    DDRD |= (1 << LED_X) | (1 << LED_Y) | (1 << LED_Z);
    PORTD &= ~((1 << LED_X) | (1 << LED_Y) | (1 << LED_Z));
}

void led_update(float x, float y, float z) {
    if (x > 20 || x < -20) PORTD |= (1 << LED_X);
    else        PORTD &= ~(1 << LED_X);

    if (y > 20 || y < -20) PORTD |= (1 << LED_Y);
    else        PORTD &= ~(1 << LED_Y);

    if (z > 20 || z < -20) PORTD |= (1 << LED_Z);
    else        PORTD &= ~(1 << LED_Z);
}
