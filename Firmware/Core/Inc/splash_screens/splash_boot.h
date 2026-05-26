#ifndef SPLASH_BOOT_H
#define SPLASH_BOOT_H

#include <stdint.h>
#include <stm32g4xx_hal.h>

typedef struct {
    uint8_t x;
    uint8_t y;
} AnimationStep;

void splash_screen_boot_play(GPIO_TypeDef* port, uint16_t pin);

#endif
