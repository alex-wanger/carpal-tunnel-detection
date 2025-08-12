#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

void debounce_init(void);

bool debounce_pressed_edge(void);

#endif
