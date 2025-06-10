#include "i2c/ina226.h"
#include "i2c/i2c.h"
#include "usb_serial.h" //debug

static uint16_t INA226_ReadRegister(uint16_t address, uint8_t reg) {
	uint8_t buffer[2];
	I2C_Read(buffer, address, reg, 2);
	return ((uint16_t)buffer[0] << 8) | buffer[1];
}

uint16_t INA226_ReadBusVoltage(void* context) {
	uint16_t address = *((uint16_t*)context);
	return INA226_ReadRegister(address, INA226_REG_BUS_VOLTAGE);
}

uint16_t INA226_ReadCurrent(void* context) {
	uint16_t address = *((uint16_t*)context);
	return INA226_ReadRegister(address, INA226_REG_CURRENT);
}
