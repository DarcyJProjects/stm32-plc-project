#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "i2c/ssd1306/ssd1306.h"
#include "i2c/ssd1306/ssd1306_fonts.h"
#include "i2c/ssd1306/ssd1306_icons.h"
#include "stm32g4xx_hal.h"
#include "splash_screens/screen0.h"
#include "main.h"

extern const unsigned char sprite_cursor_12_19 [];
extern const AnimationStep screen0_path [];
extern const uint16_t screen0_frame_count;
extern const uint8_t screen0_w;
extern const uint8_t screen0_h;
extern const uint8_t screen0_period;
const uint8_t sine_lut_64_8_ampl[64] = {
    4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8,
    8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4,
    4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4
}; // look up table for sine function

// ALL SCREENS MUST BE VERTICAL 1 BIT PER PIXEL

static void splash_screen0(void) {
	for (int i = 0; i < screen0_frame_count; i++) {
		ssd1306_Fill(Black);
		ssd1306_DrawBitmap(screen0_path[i].x, screen0_path[i].y, sprite_cursor_12_19, screen0_w, screen0_h, White);
		ssd1306_UpdateScreen();
		HAL_Delay(screen0_period);
	}
}

static void splash_screen0_end_frame(void) {
	// Keep the cursor in the ending position of screen0
	uint8_t cursor_x = screen0_path[screen0_frame_count-1].x;
	uint8_t cursor_y = screen0_path[screen0_frame_count-1].y;
	ssd1306_DrawBitmap(cursor_x, cursor_y, sprite_cursor_12_19, screen0_w, screen0_h, White);
}


static void splash_screen1(void) {
	const char* text = "DarcyJProjects";
	char buffer[20];

	// Custom timing for each character
	uint16_t char_delays[] = {
			125, 125, 250, 125, 125, 375, 250, 125, 250, 125, 125, 125, 125, 125
	};

	// Inital caret blinkging
	for (int i = 0; i < 2; i++) {
		// Blink On
		ssd1306_Fill(Black);
		ssd1306_SetCursor(24, 27);
		splash_screen0_end_frame();
		ssd1306_WriteString("_", Font_7x10, White);
		ssd1306_UpdateScreen();
		HAL_Delay(375);

		// Blink Off
		ssd1306_Fill(Black);
		ssd1306_SetCursor(24, 27);
		splash_screen0_end_frame();
		ssd1306_UpdateScreen();
		HAL_Delay(375);
	}

	// Typewriter text
	int len = strlen(text);
	for (int i = 1; i <= len; i++) {
		ssd1306_Fill(Black);
		ssd1306_SetCursor(24, 27);
		splash_screen0_end_frame();

		// Build the current string
		memset(buffer, 0, sizeof(buffer));
		strncpy(buffer, text, i);

		// Add the underscore at the end, unless its the last letter
		if (i < len) {
			strcat(buffer, "_");
		}

		ssd1306_WriteString(buffer, Font_7x10, White);
		ssd1306_UpdateScreen();

		// Timing table for delay
		HAL_Delay(char_delays[i-1]);

		// Extra hold on the J
		if (text[i-1] == 'J') {
			// Blink caret
			ssd1306_Fill(Black);
			ssd1306_SetCursor(24, 27);
			splash_screen0_end_frame();
			ssd1306_WriteString("DarcyJ", Font_7x10, White);
			ssd1306_UpdateScreen();
			HAL_Delay(375);
		}
	}
}

static void splash_screen2(void) {
	int phase = 0;
	int duration = 60; // 100 = roughly 3 seconds at 30 ms delay

	for (int frame = 0; frame < duration; frame++) {
		ssd1306_Fill(Black);

		// Text
		ssd1306_SetCursor(36, 11);
		ssd1306_WriteString("Embedded", Font_7x10, White);

		ssd1306_SetCursor(29, 23);
		ssd1306_WriteString("Controller", Font_7x10, White);

		ssd1306_SetCursor(46, 54);
		char str[16] = "v";
		strcat(str, VERSION);
		ssd1306_WriteString(str, Font_6x8, White);

		// Sine wave animation
		for (int x = 34; x < 94; x++) {
			// (x + phase) / 4 stretches the wave horizontally
			// x * 2 just decreases the wavelength

			int lut_index = (x * 2 + phase) & 63;
			int y_offset = sine_lut_64_8_ampl[lut_index];

			ssd1306_DrawPixel(x, 42 + y_offset, White);
		}

		ssd1306_UpdateScreen();
		phase += 2;
		HAL_Delay(30);
	}
}

static bool should_skip(GPIO_TypeDef* port, uint16_t pin) {
	return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
}

void splash_screen_boot_play(GPIO_TypeDef* port, uint16_t pin) {
	if (should_skip(port, pin)) return;
	splash_screen0();

	if (should_skip(port, pin)) return;
	splash_screen1();

	if (should_skip(port, pin)) return;
	HAL_Delay(1500);

	if (should_skip(port, pin)) return;
	splash_screen2();
}
