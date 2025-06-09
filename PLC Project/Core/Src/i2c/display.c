#include "i2c/display.h"
#include "i2c/ssd1306/ssd1306.h"
#include "i2c/ssd1306/ssd1306_fonts.h"
#include "modbus/modbus.h"

// Variables
static GPIO_TypeDef* BTN_PORT;
static uint16_t BTN_PIN;

void display_Setup(GPIO_TypeDef* btn_port, uint16_t btn_pin) {
	// Store button GPIO
	BTN_PORT = btn_port;
	BTN_PIN = btn_pin;

	// Initialise SSD1306
	ssd1306_Init();
}

void display_Boot(void) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(10, 20);
	ssd1306_WriteString("PLC by DJ", Font_11x18, White);
	ssd1306_SetCursor(25, 45);
	ssd1306_WriteString("Booting...", Font_7x10, White);
	ssd1306_UpdateScreen();
}

void display_StatusPage(void) {
	char buf[32]; // buffer for formatted strings

	ssd1306_Fill(Black);
	ssd1306_SetCursor(25, 0);
	ssd1306_WriteString("Status", Font_11x18, White);

	ssd1306_SetCursor(2, 25);
	sprintf(buf, "Slave Add: 0x%02X", modbusGetSlaveAddress());
	ssd1306_WriteString(buf, Font_6x8, White);

	ssd1306_SetCursor(2, 40);
	sprintf(buf, "Supply Voltage: %dV", 5);
	ssd1306_WriteString(buf, Font_6x8, White);

	ssd1306_SetCursor(2, 55);
	sprintf(buf, "Current Draw: %dmA", 50);
	ssd1306_WriteString(buf, Font_6x8, White);


	ssd1306_UpdateScreen();
}
