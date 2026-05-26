#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"

bool EEPROM_Dump(void);
bool EEPROM_Reflash(const uint8_t* data, uint16_t len);
bool EEPROM_Wipe(uint16_t upToAddr);


#endif
