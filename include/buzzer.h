#ifndef BUZZER_H
#define BUZZER_H

#include <avr/io.h>
#include <stdbool.h>
#ifndef BUZZER_DDR
#define BUZZER_DDR   DDRB
#endif
#ifndef BUZZER_PORT
#define BUZZER_PORT  PORTB
#endif
#ifndef BUZZER_BIT
#define BUZZER_BIT   PB1
#endif
#ifndef BUZZER_ACTIVE_HIGH
#define BUZZER_ACTIVE_HIGH 1 
#endif
#ifndef BUZZ_THR_X_NEG
#define BUZZ_THR_X_NEG  (-28.0f)
#endif
#ifndef BUZZ_THR_X_POS
#define BUZZ_THR_X_POS  ( 33.0f)
#endif
#ifndef BUZZ_THR_Y_POS
#define BUZZ_THR_Y_POS  ( 53.0f)
#endif
#ifndef BUZZ_THR_Y_NEG
#define BUZZ_THR_Y_NEG  (-53.0f)
#endif
#ifndef BUZZ_THR_Z_POS
#define BUZZ_THR_Z_POS  ( 16.0f)
#endif
#ifndef BUZZ_THR_Z_NEG
#define BUZZ_THR_Z_NEG  (-16.0f)
#endif

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
bool buzzer_should_beep(float x, float y, float z);
void buzzer_update(float x, float y, float z);

#endif
