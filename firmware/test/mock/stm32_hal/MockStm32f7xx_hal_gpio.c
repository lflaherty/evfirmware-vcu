/*
 * MockStm32f7xx_hal_gpio.c
 *
 *  Created on: 19 Jul 2022
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_gpio.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>

// ------------------- Static data -------------------
#define MAX_NUM_PINS 32
static struct mockPin
{
    GPIO_TypeDef* refGPIOx; // Pointer to GPIOx to match against
    uint16_t GPIO_Pin; // pin to match against
    bool inUse;

    bool asserted;
} pinStore[MAX_NUM_PINS] = { 0 };
size_t numPins = 0U;

// Returns NULL if not found
struct mockPin* getMockPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    for (size_t i = 0; i < numPins; ++i) {
        struct mockPin* p = &pinStore[i];
        if (p->inUse && p->refGPIOx == GPIOx && p->GPIO_Pin == GPIO_Pin) {
            return p;
        }
    }

    return NULL;
}

// ------------------- Methods -------------------
GPIO_PinState stubHAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    struct mockPin* p = getMockPin(GPIOx, GPIO_Pin);
    assert(p != NULL);

    return p->asserted ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void stubHAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    struct mockPin* p = getMockPin(GPIOx, GPIO_Pin);
    assert(p != NULL);

    p->asserted = (PinState == GPIO_PIN_SET);
}

void stubHAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    struct mockPin* p = getMockPin(GPIOx, GPIO_Pin);
    assert(p != NULL);

    p->asserted = !p->asserted;
}

void mock_GPIO_RegisterPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    // Make sure pin not already registered
    assert(numPins < MAX_NUM_PINS);

    // Make sure we aren't overriding an already registered pin
    struct mockPin* p = getMockPin(GPIOx, GPIO_Pin);
    if (NULL != p) {
        assert(p->refGPIOx == GPIOx);
        assert(p->GPIO_Pin == GPIO_Pin);

        // Already registered
        return;
    }

    pinStore[numPins].refGPIOx = GPIOx;
    pinStore[numPins].GPIO_Pin = GPIO_Pin;
    pinStore[numPins].inUse = true;
    numPins++;
}

void mockSet_GPIO_Asserted(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool asserted)
{
    struct mockPin* p = getMockPin(GPIOx, GPIO_Pin);
    assert(p != NULL);

    p->asserted = asserted;
}

bool mockGet_GPIO_Asserted(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    struct mockPin* p = getMockPin(GPIOx, GPIO_Pin);
    assert(p != NULL);

    return p->asserted;
}
