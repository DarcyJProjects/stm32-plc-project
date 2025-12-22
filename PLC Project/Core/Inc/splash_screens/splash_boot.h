#ifndef SPLASH_BOOT_H
#define SPLASH_BOOT_H

#include <stdint.h>

typedef struct {
    uint8_t x;
    uint8_t y;
} AnimationStep;

void splash_screen_boot_play(void);

#endif
