#include "i2c/i2c.h"

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
	// Tell the I2C device which register to read
	I2C_Transmit(address << 1, &reg, 1);

	// Read however many bytes from that register
	I2C_Receive((address << 1) | 1, buffer, bytes);
}

