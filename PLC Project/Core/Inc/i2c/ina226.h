#ifndef INA226_H
#define INA226_H

#include "stm32g4xx_hal.h"

// INA226 I2C register addresses
#define INA226_REG_BUS_VOLTAGE 0x02
#define INA226_REG_CURRENT 0x04

// I2C Address is 0x40

// Reads raw 16-bit voltage (1.25 mV per bit)
uint16_t INA226_ReadBusVoltage(void* context);

// Reads raw 16-bit current (scaled based on calibration)
uint16_t INA226_ReadCurrent(void* context);

#endif
