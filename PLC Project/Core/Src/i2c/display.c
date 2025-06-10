#include "i2c/display.h"
#include "i2c/ssd1306/ssd1306.h"
#include "i2c/ssd1306/ssd1306_fonts.h"
#include "modbus/modbus.h"
#include "i2c/ina226.h"

#include "io/io_coils.h"

// Variables
uint16_t currentPage = 0;
uint16_t endPage = 1;

void display_Setup() {
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
	switch(currentPage) {
		case 0:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(25, 0);
			ssd1306_WriteString("Status", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "Slave Add: 0x%02X", modbusGetSlaveAddress());
			ssd1306_WriteString(buf, Font_6x8, White);

			uint16_t ina226_address = 0x40; // 7-bit I2C address

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "Supply Voltage: %dV", INA226_ReadBusVoltage(&ina226_address));
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 55);
			sprintf(buf, "Current Draw: %dmA", INA226_ReadCurrent(&ina226_address));
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
		case 1:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(30, 0);
			ssd1306_WriteString("Coils", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "0: %s", io_coil_read(0) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "1: %s", io_coil_read(1) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 25);
			sprintf(buf, "2: %s", io_coil_read(2) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 40);
			sprintf(buf, "3: %s", io_coil_read(3) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
	}

	ssd1306_UpdateScreen();
}

void display_BtnPress() {
	if (currentPage == endPage) {
		currentPage = 0;
	}
	else {
		currentPage++;
	}

	display_StatusPage();
}
