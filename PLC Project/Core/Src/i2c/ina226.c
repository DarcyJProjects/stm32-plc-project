#include "i2c/ina226.h"
#include "i2c/i2c.h"

I2C_HandleTypeDef handle;

// Static Functions
static void INA226_WriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t data[3];
    data[0] = reg;
    data[1] = (value >> 8) & 0xFF; // MSB
    data[2] = value & 0xFF;        // LSB
    I2C_Transmit(INA226_I2C_ADDR << 1, data, 3);
}

static uint16_t INA226_ReadRegister(uint8_t reg)
{
    uint8_t data[2];

    I2C_Read(data, INA226_I2C_ADDR, reg, 2);
    return (data[0] << 8) | data[1];
}

// Functions

void INA226_Init(I2C_HandleTypeDef *hi2c) {
	// Store I2C handle
	handle = *hi2c;

	// Write configuration register
	INA226_WriteRegister(INA226_REG_CONFIG, INA226_CONFIG_DEFAULT);

	// Write calibration value
	INA226_WriteRegister(INA226_REG_CALIBRATION, INA226_CALIBRATION_VALUE);
}

uint16_t INA226_ReadBusVoltageRaw(void) {
	return INA226_ReadRegister(INA226_REG_BUS_VOLT);
}

float INA226_ReadBusVoltage(void) {
	return INA226_ReadBusVoltageRaw() * 0.00125f; // 1.25mV per LSB
}

uint16_t INA226_ReadCurrentRaw(void) {
	return INA226_ReadRegister(INA226_REG_CURRENT);
}

float INA226_ReadCurrent(void) {
	return INA226_ReadCurrentRaw() * 0.00015f; // 150uA per LSB with calibration value 0x0155
}

uint16_t INA226_ReadPowerRaw(void) {
	return INA226_ReadRegister(INA226_REG_POWER);
}

float INA226_ReadPower(void) {
	return INA226_ReadPowerRaw() * 0.00375f; // 3.75mW per LSB with calibration value 0x0155
}
