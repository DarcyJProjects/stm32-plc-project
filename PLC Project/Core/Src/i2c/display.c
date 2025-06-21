#include <i2c/ina226.h>
#include "i2c/display.h"
#include "i2c/ssd1306/ssd1306.h"
#include "i2c/ssd1306/ssd1306_fonts.h"
#include "i2c/ssd1306/ssd1306_icons.h"
#include "modbus/modbus.h"
#include "io/io_coils.h"
#include "rtc/rtc_ds3231.h"

// Variables
uint16_t currentPage = 0;
uint16_t endPage = 5;

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
			sprintf(buf, "Slave Addr: 0x%02X", modbusGetSlaveAddress());
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			float voltage = INA226_ReadBusVoltage();
			if (voltage > 10) sprintf(buf, "Supply: %.1fV", voltage); // only 1dp will fit
			else sprintf(buf, "Supply: %.2fV", voltage); // 2dp will fit
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 55);
			sprintf(buf, "Current: %.0fmA", INA226_ReadCurrent() * 1000);
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
		case 2:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(4, 0);
			ssd1306_WriteString("Discrete In", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "0: %s", io_discrete_in_read(0) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "1: %s", io_discrete_in_read(1) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 25);
			sprintf(buf, "2: %s", io_discrete_in_read(2) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 40);
			sprintf(buf, "3: %s", io_discrete_in_read(3) ? "ON" : "OFF");
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
		case 3:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(2, 0);
			ssd1306_WriteString("Holding Reg", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "0: %d", io_holding_reg_read(0));
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "1: %d", io_holding_reg_read(1));
			ssd1306_WriteString(buf, Font_6x8, White);

			/*ssd1306_SetCursor(60, 25);
			sprintf(buf, "2: %d", io_holding_reg_read(2));
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 40);
			sprintf(buf, "3: %d", io_holding_reg_read(3));
			ssd1306_WriteString(buf, Font_6x8, White);*/
			break;
		case 4:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(12, 0);
			ssd1306_WriteString("Input Reg", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "0: %d", io_input_reg_read(0));
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "1: %d", io_input_reg_read(1));
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 25);
			sprintf(buf, "2: %d", io_input_reg_read(2));
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 40);
			sprintf(buf, "3: %d", io_input_reg_read(3));
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
		case 5:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(50, 0);
			ssd1306_WriteString("RTC", Font_11x18, White);

			RTC_Time current;
			DS3231_ReadTime(&current);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "%02d:%02d:%02d", current.hours, current.minutes, current.seconds);
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "%02d/%02d/20%02d", current.day, current.month, current.year);
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
	}

	ssd1306_SetCursor(110, 56);
	sprintf(buf, "%d/%d", currentPage, endPage);
	ssd1306_WriteString(buf, Font_6x8, White);

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

void display_setPage(uint16_t page) {
	if (page > endPage) return;
	currentPage = page;
}
