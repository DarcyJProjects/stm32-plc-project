#ifndef EEPROM_H
#define EEPROM_H

#include "stm32g4xx_hal.h"
#include "i2c/i2c.h"
#include <stdbool.h>
#include <string.h>

#define EEPROM_I2C_ADDR		0x57
#define EEPROM_PAGE_SIZE	32
#define EEPROM_SIZE_BYTES	4096


/*
 * RULES ADDRESS IS 0x0000
 * VIRTUAL REGISTERS START IMMEDIATELY AFTER RULES CRC16 (DYNAMIC ADDRESS)
 */

bool EEPROM_Write(uint16_t memAddr, uint8_t* data, uint16_t len);

bool EEPROM_Read(uint16_t memAddr, uint8_t* data, uint16_t len);


bool EEPROM_WriteBlock(uint16_t memAddr, const void* data, uint16_t len);

bool EEPROM_LoadBlock(uint16_t memAddr, void* data, uint16_t len);

#endif
