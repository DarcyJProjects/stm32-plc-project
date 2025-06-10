// Not part of original library, added by me
#ifndef SSD1306_ICONS_H
#define SSD1306_ICONS_H

#include <stdint.h>
#include "i2c/ssd1306/ssd1306.h"

typedef enum {
    ICON_CIRCLE_EMPTY,
    ICON_CIRCLE_FILLED
} IconType;

typedef enum {
	Icon_6x8,
	Icon_11x18
} IconSize;

// Draws a 6x8 icon at (x, y) in pixels
void ssd1306_DrawIcon(IconSize size, IconType icon, uint8_t x, uint8_t y);

#endif
