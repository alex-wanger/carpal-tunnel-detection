#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "timer.h"

volatile uint32_t timer0_millis = 0;
volatile uint16_t timer0_frac   = 0;

ISR(TIMER0_OVF_vect)
{
    timer0_millis += 1;
    timer0_frac   += 24;
    if (timer0_frac >= 1000) {
        timer0_frac -= 1000;
        timer0_millis += 1;
    }
}

void timer0_init(void)
{
    TCCR0A = 0;
    TCCR0B = (1 << CS01) | (1 << CS00);
    TIMSK0 = (1 << TOIE0);
    sei();
}

uint32_t millis(void)
{
    timer0_init();
    uint32_t m;
    uint8_t oldSREG = SREG;
    cli();
    m = timer0_millis;
    SREG = oldSREG;
    return m;
}
