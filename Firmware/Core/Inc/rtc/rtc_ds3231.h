// TODO: Migrate I2C abstraction module (i2c/i2c.h) over direct HAL

#ifndef RTC_DS3231_H
#define RTC_DS3231_H

#include "stm32g4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define DS3231_ADDRESS (0x68 << 1) // 8 bit address

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day_of_week;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} RTC_Time;


// Functions
HAL_StatusTypeDef DS3231_ReadTime(RTC_Time* time);

HAL_StatusTypeDef DS3231_SetTime(RTC_Time* time);

uint16_t DS3231_ReadTemp(void* context); // Modbus io compatible

#endif
