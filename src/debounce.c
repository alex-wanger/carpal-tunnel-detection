#include "debounce.h"
#include <avr/io.h>
#include "twi.h"
#include "debounce.h"
#include <avr/io.h>
#include "timer.h"


#define BUTTON_PIN   PIND
#define BUTTON_BIT   PIND2
#define DEBOUNCE_MS  50

static unsigned long lastDebounceTime = 0;
static bool lastButtonReading = false;
static bool buttonState = false;

void debounce_init(void) {
    DDRD &= ~(1 << DDD2);
    PORTD |= (1 << PORTD2);
    lastButtonReading = ((BUTTON_PIN & (1 << BUTTON_BIT)) == 0);
    buttonState = lastButtonReading;
}

bool debounce_pressed_edge(void) {
    bool reading = ((BUTTON_PIN & (1 << BUTTON_BIT)) == 0);

    if (reading != lastButtonReading) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState) {
                lastButtonReading = reading;
                return true;
            }
        }
    }

    lastButtonReading = reading;
    return false;
}
