#include "simpson.h"
#include <stdlib.h>
#include "simpson.h"
#include "../include/twi.h"
#include "../include/timer.h"

static uint8_t grow_if_needed(simpson_store_t *s, uint16_t need_samples)
{
    if (s->capacity >= need_samples)
        return 1;

    uint16_t newcap = s->capacity ? (uint16_t)(s->capacity * 2u) : (uint16_t)SIMPSON_START_CAP;
    while (newcap < need_samples)
    {
        uint16_t prev = newcap;
        newcap = (uint16_t)(newcap * 2u);
        if (newcap <= prev)
        {
            newcap = need_samples;
            break;
        }
    }

    float *nx = (float *)malloc((size_t)newcap * sizeof(float));
    float *ny = (float *)malloc((size_t)newcap * sizeof(float));
    float *nz = (float *)malloc((size_t)newcap * sizeof(float));
    if (!nx || !ny || !nz)
    {
        free(nx);
        free(ny);
        free(nz);
        return 0;
    }

    for (uint16_t i = 0; i < s->size; ++i)
    {
        if (s->x)
            nx[i] = s->x[i];
        if (s->y)
            ny[i] = s->y[i];
        if (s->z)
            nz[i] = s->z[i];
    }

    free(s->x);
    free(s->y);
    free(s->z);
    s->x = nx;
    s->y = ny;
    s->z = nz;
    s->capacity = newcap;
    return 1;
}

void simpson_init(simpson_store_t *s)
{
    s->x = s->y = s->z = NULL;
    s->size = 0;
    s->capacity = 0;
}

void simpson_clear(simpson_store_t *s)
{
    s->size = 0;
}

void simpson_free(simpson_store_t *s)
{
    free(s->x);
    free(s->y);
    free(s->z);
    s->x = s->y = s->z = NULL;
    s->size = 0;
    s->capacity = 0;
}

uint8_t simpson_append(simpson_store_t *s, float gx, float gy, float gz)
{
    if (!grow_if_needed(s, (uint16_t)(s->size + 1u)))
        return 0;
    s->x[s->size] = gx;
    s->y[s->size] = gy;
    s->z[s->size] = gz;
    s->size++;
    return 1;
}

void simpson_release_on_press(simpson_store_t *s, uint8_t pressed)
{
    static uint8_t prevStable = 0;
    static uint8_t lastReading = 0;
    static unsigned long lastChangeTime = 0;
    const unsigned long debounceDelay = 50;

    if (pressed != lastReading)
    {
        lastChangeTime = millis();
    }

    if ((millis() - lastChangeTime) > debounceDelay)
    {
        if (pressed != prevStable)
        {
            prevStable = pressed;
            if (prevStable)
            {
                simpson_clear(s);
            }
        }
    }

    lastReading = pressed;
}