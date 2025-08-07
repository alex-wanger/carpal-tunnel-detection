#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h> // for dtostrf

void usart_init(uint32_t baud) {
    // enable double speed for tighter baud error
    UCSR0A |= (1<<U2X0);
    uint16_t ubrr = F_CPU/8/baud - 1;
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;
    UCSR0B = (1<<TXEN0);
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); // 8N1
}

void usart_transmit(uint8_t data) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void usart_print(const char *str) {
    while (*str) {
        usart_transmit(*str++);
    }
}

void usart_print_float(float f, uint8_t decimals) {
    char buf[16];
    dtostrf(f, decimals+3, decimals, buf);
    usart_print(buf);
}



