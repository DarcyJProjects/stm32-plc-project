#ifndef I2C_H
#define I2C_H

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_i2c.h"

void I2C_Setup(I2C_HandleTypeDef* handle);

HAL_StatusTypeDef I2C_Transmit(uint16_t address, uint8_t* data, uint16_t len);

HAL_StatusTypeDef I2C_Receive(uint16_t address, uint8_t* data, uint16_t len);

void I2C_Read(uint8_t* buffer, uint16_t address, uint8_t reg, uint16_t bytes);

#endif
