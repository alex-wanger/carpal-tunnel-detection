#include "buzzer.h"

static inline void buzzer_set(bool on)
{
#if BUZZER_ACTIVE_HIGH
    if (on)
        BUZZER_PORT |= (1 << BUZZER_BIT);
    else
        BUZZER_PORT &= ~(1 << BUZZER_BIT);
#else
    if (on)
        BUZZER_PORT &= ~(1 << BUZZER_BIT);
    else
        BUZZER_PORT |= (1 << BUZZER_BIT);
#endif
}

void buzzer_init(void)
{
    BUZZER_DDR |= (1 << BUZZER_BIT);
#if BUZZER_ACTIVE_HIGH
    BUZZER_PORT &= ~(1 << BUZZER_BIT);
#else
    BUZZER_PORT |= (1 << BUZZER_BIT);
#endif
}

void buzzer_on(void) { buzzer_set(true); }
void buzzer_off(void) { buzzer_set(false); }

bool buzzer_should_beep(float x, float y, float z)
{
    if (x < BUZZ_THR_X_NEG)
        return true;
    if (x > BUZZ_THR_X_POS)
        return true;
    if (y > BUZZ_THR_Y_POS)
        return true;
    if (y < BUZZ_THR_Y_NEG)
        return true;
    if (z > BUZZ_THR_Z_POS)
        return true;
    if (z < BUZZ_THR_Z_NEG)
        return true;
    return false;
}

void buzzer_update(float x, float y, float z)
{
    buzzer_set(buzzer_should_beep(x, y, z));
}
