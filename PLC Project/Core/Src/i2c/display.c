#include <i2c/ina226.h>
#include <modbus/modbus_slave.h>
#include "i2c/display.h"
#include "i2c/ssd1306/ssd1306.h"
#include "i2c/ssd1306/ssd1306_fonts.h"
#include "i2c/ssd1306/ssd1306_icons.h"
#include "io/io_coils.h"
#include "rtc/rtc_ds3231.h"
#include "io/io_virtual.h"
#include "sd/sd.h"


// Variables
uint16_t currentPage = 0;
uint16_t endPage = 10;

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
			ssd1306_SetCursor(2, 0);
			ssd1306_WriteString("Holding Reg", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			IO_Holding_Reg_Mode mode0hr;
			io_holding_reg_get_mode(0, &mode0hr);
			sprintf(buf, "0: %s", mode0hr == IO_HOLDING_REG_VOLTAGE ? "0-5V" : "4-20mA");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			IO_Holding_Reg_Mode mode1hr;
			io_holding_reg_get_mode(1, &mode1hr);
			sprintf(buf, "1: %s", mode1hr == IO_HOLDING_REG_VOLTAGE ? "0-5V" : "4-20mA");
			ssd1306_WriteString(buf, Font_6x8, White);

			break;
		case 5:
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
		case 6:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(12, 0);
			ssd1306_WriteString("Input Reg", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			IO_Input_Reg_Mode mode0ir;
			io_input_reg_get_mode(0, &mode0ir);
			sprintf(buf, "0: %s", mode0ir == IO_INPUT_REG_VOLTAGE ? "0-5V" : "4-20mA");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			IO_Input_Reg_Mode mode1ir;
			io_input_reg_get_mode(1, &mode1ir);
			sprintf(buf, "1: %s", mode1ir == IO_INPUT_REG_VOLTAGE ? "0-5V" : "4-20mA");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 25);
			IO_Input_Reg_Mode mode2ir;
			io_input_reg_get_mode(2, &mode2ir);
			sprintf(buf, "2: %s", mode2ir == IO_INPUT_REG_VOLTAGE ? "0-5V" : "4-20mA");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(60, 40);
			IO_Input_Reg_Mode mode3ir;
			io_input_reg_get_mode(3, &mode3ir);
			sprintf(buf, "3: %s", mode3ir == IO_INPUT_REG_VOLTAGE ? "0-5V" : "4-20mA");
			ssd1306_WriteString(buf, Font_6x8, White);

			break;
		case 7:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(7, 0);
			ssd1306_WriteString("Automation", Font_11x18, White);

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "%d rules", automation_get_rule_count());
			ssd1306_WriteString(buf, Font_6x8, White);

			uint16_t virtCoils = 0;
			uint16_t virtHolding = 0;
			io_virtual_get_count(VIR_COIL, &virtCoils);
			io_virtual_get_count(VIR_HOLDING, &virtHolding);

			ssd1306_SetCursor(2, 40);
			sprintf(buf, "%d virt. coils", virtCoils);
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 55);
			sprintf(buf, "%d virt. holding", virtHolding);
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
		case 8:
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
		case 9:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(5, 0);
			ssd1306_WriteString("Emerg. Stop", Font_11x18, White);

			bool defined = emergencyStop_isDefined();

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "Enabled: %s", defined ? "true" : "false");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			if (defined) {
			    sprintf(buf, "D. Input: %u", emergencyStop_getChannel());
			} else {
			    sprintf(buf, "D. Input: -");
			}
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 55);
			sprintf(buf, "Input Mode: %s", defined ? (emergencyStop_getInputMode() == EMERGENCY_STOP_NO ? "NO" : "NC") : "-");
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
		case 10:
			ssd1306_Fill(Black);
			ssd1306_SetCursor(5, 0);
			ssd1306_WriteString("SD Logging", Font_11x18, White);

			bool isMounted = SD_IsMounted();

			SD_Stats stats = SD_GetStats();

			ssd1306_SetCursor(2, 25);
			sprintf(buf, "Mounted: %s", isMounted ? "true" : "false");
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			if (stats.success) {
				sprintf(buf, "Size: %luMB", stats.totalMB);
			} else {
				sprintf(buf, "Size: -");
			}
			ssd1306_WriteString(buf, Font_6x8, White);

			ssd1306_SetCursor(2, 55);
			if (stats.success) {
				sprintf(buf, "Free: %luMB", stats.freeMB);
			} else {
				sprintf(buf, "Free: -");
			}
			ssd1306_WriteString(buf, Font_6x8, White);
			break;
	}


	if (currentPage >= 10) {
		ssd1306_SetCursor(95, 56);
	} else {
		ssd1306_SetCursor(100, 56);
	}
	sprintf(buf, "%d/%d", currentPage, endPage);
	ssd1306_WriteString(buf, Font_6x8, White);

	ssd1306_UpdateScreen();
}

void display_FactoryResetPage(uint8_t page) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(25, 0);
	ssd1306_WriteString("RESET", Font_11x18, White);

	switch (page) {
		case 0:
			ssd1306_SetCursor(2, 25);
			ssd1306_WriteString("Continue holding", Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			ssd1306_WriteString("for 5 seconds.", Font_6x8, White);
			break;
		case 1:
			ssd1306_SetCursor(2, 25);
			ssd1306_WriteString("Reset successful.", Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			ssd1306_WriteString("Booting...", Font_6x8, White);
			break;
		case 2:
			ssd1306_SetCursor(2, 25);
			ssd1306_WriteString("Reset unsuccessful!", Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			ssd1306_WriteString("Booting...", Font_6x8, White);
			break;
		case 3:
			ssd1306_SetCursor(2, 25);
			ssd1306_WriteString("Reset cancelled.", Font_6x8, White);

			ssd1306_SetCursor(2, 40);
			ssd1306_WriteString("Booting...", Font_6x8, White);
			break;
	}

	ssd1306_UpdateScreen();
}

void display_EmergencyStop(void) {
	ssd1306_Fill(Black);
	ssd1306_SetCursor(5, 0);
	ssd1306_WriteString("EMERGENCY", Font_11x18, White);

	ssd1306_SetCursor(2, 25);
	ssd1306_WriteString("All outputs set OFF.", Font_6x8, White);

	ssd1306_SetCursor(2, 40);
	ssd1306_WriteString("Reboot to reset.", Font_6x8, White);

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
