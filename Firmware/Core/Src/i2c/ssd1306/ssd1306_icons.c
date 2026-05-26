#include "i2c/ssd1306/ssd1306_icons.h"

static const uint8_t circle_filled_6x8[6][1] = {
    { 0b00111100 },
    { 0b01111110 },
    { 0b01111110 },
	{ 0b01111110 },
	{ 0b01111110 },
	{ 0b00111100 }
};

static const uint8_t circle_empty_6x8[6][1] = {
	{ 0b00111100 },
	{ 0b01100110 },
	{ 0b01000010 },
	{ 0b01000010 },
	{ 0b01100110 },
	{ 0b00111100 }
};


typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t bytes_per_column;
} IconDimensions;


static IconDimensions get_icon_dimensions(IconSize size) {
	IconDimensions dim = {0};
	switch (size) {
		case Icon_6x8:
			dim.width = 6;
			dim.height = 8;
			dim.bytes_per_column = 1;
			break;
	}
	return dim;
}

static const void* get_icon_bitmap(IconSize size, IconType icon) {
    switch (size) {
    	case Icon_6x8:
    		switch (icon) {
    			case ICON_CIRCLE_FILLED: return circle_filled_6x8;
				case ICON_CIRCLE_EMPTY: return circle_empty_6x8;
				default: return NULL;
    		}
		default:
			return NULL;
    }
}

void ssd1306_DrawIcon(IconSize size, IconType icon, uint8_t x, uint8_t y) {
	const uint8_t* bitmap = get_icon_bitmap(size, icon);
	if (!bitmap) return;

	IconDimensions dim = get_icon_dimensions(size);
	if (dim.width == 0 || dim.height == 0) return;

    for (uint8_t col = 0; col < dim.width; col++) {
    	for (uint8_t byte = 0; byte < dim.bytes_per_column; byte++) {
    		uint8_t data = bitmap[col * dim.bytes_per_column + byte];
    		uint8_t row_start = byte * 8;
    		uint8_t row_end = (byte + 1) * 8;
    		if (row_end > dim.height) row_end = dim.height;

    		for (uint8_t row = row_start; row < row_end; row++) {
    			if (data & (1 << (row % 8))) {
    				ssd1306_DrawPixel(x + col, y + row, White);
    			} else {
    				ssd1306_DrawPixel(x + col, y + row, Black);
    			}
    		}
    	}
    }
}

