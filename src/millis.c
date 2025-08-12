#include "millis.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile unsigned long g_ms = 0;

void millis_init(void) {
    TCCR0A = (1 << WGM01);
    TCCR0B = 0;
    OCR0A  = 249;
    TCNT0  = 0;
    TIMSK0 = (1 << OCIE0A);
    TCCR0B = (1 << CS01) | (1 << CS00);
    sei();
}

ISR(TIMER0_COMPA_vect) {
    g_ms++;
}

unsigned long millis(void) {
    uint8_t s = SREG; cli();
    unsigned long v = g_ms;
    SREG = s;
    return v;
}


