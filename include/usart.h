#ifndef USART_H
#define USART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void usart_init(uint32_t baud);
void usart_transmit(uint8_t data);
void usart_print(const char *str);
void usart_print_float(float f, uint8_t decimals);

#ifdef __cplusplus
}
#endif

#endif // USART_H
