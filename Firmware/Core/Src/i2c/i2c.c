#include "i2c/i2c.h"
#include <stdio.h>
#include "usb_serial.h"

#define PRINT_DEBUG

static I2C_HandleTypeDef* hi2c;

void I2C_Setup(I2C_HandleTypeDef* handle) {
	hi2c = handle;
}


HAL_StatusTypeDef I2C_Transmit(uint16_t address, uint8_t* data, uint16_t len) {
	return HAL_I2C_Master_Transmit(hi2c, address, data, len, HAL_MAX_DELAY);
}

HAL_StatusTypeDef I2C_Receive(uint16_t address, uint8_t* data, uint16_t len) {
	return HAL_I2C_Master_Receive(hi2c, address, data, len, HAL_MAX_DELAY);
}

void I2C_Read(uint8_t* buffer, uint16_t address, uint8_t reg, uint16_t bytes) {
	HAL_StatusTypeDef status;

	// Tell the I2C device which register to read
	status = I2C_Transmit(address << 1, &reg, 1);
#ifdef PRINT_DEBUG
	if (status != HAL_OK) {
		char msg[32];
		snprintf(msg, sizeof(msg), "I2C_Transmit failed: %d", status);
		usb_serial_println(msg);
		return;
	}
#endif

	// Read however many bytes from that register
	status = I2C_Receive((address << 1) | 1, buffer, bytes);
#ifdef PRINT_DEBUG
	if (status != HAL_OK) {
		char msg[32];
		snprintf(msg, sizeof(msg), "I2C_Receive failed: %d", status);
		usb_serial_println(msg);
		return;
	}
#endif
}

